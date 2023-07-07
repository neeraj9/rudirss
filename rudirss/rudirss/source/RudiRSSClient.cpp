#include "RudiRSSClient.h"
#include "FeedBase.h"
#include "FeedCommon.h"

#include <userenv.h>
#include <shlobj_core.h>

RudiRSSClient::RudiRSSClient() : m_dbSemaphore{ nullptr }, m_dbStopEvent{ nullptr }, m_dbConsumptionThread{ nullptr }
{
    m_dbLock.Init();
    m_dbSemaphore = CreateSemaphore(nullptr, 0, DEFAULT_MAX_CONSUMPTION_COUNT, nullptr);
    m_dbStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
}

RudiRSSClient::~RudiRSSClient()
{
    StopDBConsumption();

    CloseHandle(m_dbSemaphore);
    CloseHandle(m_dbStopEvent);
}

bool RudiRSSClient::Initialize()
{
    try
    {
        WCHAR appDataDir[MAX_PATH]{};
        SHGetSpecialFolderPath(NULL, appDataDir, CSIDL_APPDATA, FALSE);
        std::wstring dbPath = std::wstring(appDataDir) + L"\\rudirss.db";
        m_db.Open(dbPath);
        m_db.Initialize();
    }
    catch (const std::exception &e)
    {
        return false;
    }

    StartDBConsumption();

    return FeedClient::Initialize();
}

void RudiRSSClient::OnFeedReady(const std::unique_ptr<Feed>& feed)
{
    if (!feed)
        return;

    PushDBConsumptionUnit(feed);
}

unsigned __stdcall RudiRSSClient::ThreadDBConsumption(void* param)
{
    auto pThis = reinterpret_cast<RudiRSSClient*>(param);
    pThis->DBConsumption();
    return 0;
}

void RudiRSSClient::DBConsumption()
{
    while (WAIT_OBJECT_0 != WaitForSingleObject(m_dbStopEvent, 0))
    {
        WaitForSingleObject(m_dbSemaphore, INFINITE);
        if (WAIT_OBJECT_0 == WaitForSingleObject(m_dbStopEvent, 0))
            break;

        FeedDatabase::FeedConsumptionUnit consumptionUnit;
        if (!PopDBConsumptionUnit(consumptionUnit))
            continue;

        m_db.InsertFeed(consumptionUnit.feed);
        for (const auto& feedData : consumptionUnit.feedDataContainer)
        {
            m_db.InsertFeedData(feedData);
        }
    }
}

void RudiRSSClient::StartDBConsumption()
{
    StopDBConsumption();

    ResetEvent(m_dbStopEvent);
    if (!m_dbConsumptionThread)
    {
        m_dbConsumptionThread = (HANDLE)_beginthreadex(nullptr, 0, ThreadDBConsumption, this, 0, nullptr);
    }
}

void RudiRSSClient::StopDBConsumption()
{
    if (m_dbConsumptionThread)
    {
        SetEvent(m_dbStopEvent);
        ::ReleaseSemaphore(m_dbSemaphore, 1, nullptr);
        WaitForSingleObject(m_dbConsumptionThread, INFINITE);
        CloseHandle(m_dbConsumptionThread);
        m_dbConsumptionThread = nullptr;
    }
}

void RudiRSSClient::PushDBConsumptionUnit(const std::unique_ptr<Feed>& feed)
{
    FeedBase* feedBase = reinterpret_cast<FeedBase*>(feed.get());
    auto spec = feedBase->GetSpec();
    FeedDatabase::FeedConsumptionUnit consumptionUnit;
    FeedCommon::ConvertWideStringToString(feed->GetValue(FeedCommon::FeedSpecification::RSS == spec ? L"link" : L"id"), consumptionUnit.feed.guid);
    consumptionUnit.feed.url = consumptionUnit.feed.guid;
    FeedCommon::ConvertWideStringToString(feed->GetValue(L"title"), consumptionUnit.feed.title);

    feed->IterateFeeds([&](const FeedData& feedData) -> bool {
        FeedDatabase::FeedData dbFeedData;
        FeedCommon::ConvertWideStringToString(feedData.GetValue(FeedCommon::FeedSpecification::RSS == spec ? L"link" : L"id"), dbFeedData.guid);
        dbFeedData.feedguid = consumptionUnit.feed.guid;
        dbFeedData.link = dbFeedData.guid;
        FeedCommon::ConvertWideStringToString(feedData.GetValue(L"title"), dbFeedData.title);
        FeedCommon::ConvertWideStringToString(feedData.GetValue(FeedCommon::FeedSpecification::RSS == spec ? L"pubDate" : L"updated"), dbFeedData.datetime);
        dbFeedData.timestamp = 0;
        dbFeedData.createdtime = time(nullptr);
        dbFeedData.tag = "";
        dbFeedData.misc = "";
        consumptionUnit.feedDataContainer.push_back(std::move(dbFeedData));
        return true;
        });

    {
        ATL::CComCritSecLock lock(m_dbLock);
        m_dbQueue.push(std::move(consumptionUnit));
    }
    ::ReleaseSemaphore(m_dbSemaphore, 1, nullptr);
}

bool RudiRSSClient::PopDBConsumptionUnit(FeedDatabase::FeedConsumptionUnit& consumptionUnit)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (m_dbQueue.empty())
        return false;

    consumptionUnit = std::move(m_dbQueue.front());
    m_dbQueue.pop();

    return true;
}

bool RudiRSSClient::QueryFeed(long long feedId, FeedDatabase::FN_QUERY_FEED fnQueryFeed)
{
    return m_db.QueryFeed(feedId, fnQueryFeed);
}

bool RudiRSSClient::QueryAllFeeds(FeedDatabase::FN_QUERY_FEED fnQueryFeed)
{
    return m_db.QueryAllFeeds(fnQueryFeed);
}

bool RudiRSSClient::QueryFeedData(const std::string& guid, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData)
{
    return m_db.QueryFeedData(guid, fnQueryFeedData);
}

bool RudiRSSClient::QueryFeedData(long long feeddataid, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData)
{
    return m_db.QueryFeedData(feeddataid, fnQueryFeedData);
}

