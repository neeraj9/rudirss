#include "RudiRSSClient.h"
#include "FeedBase.h"
#include "FeedCommon.h"

#include <userenv.h>
#include <shlobj_core.h>
#include <format>

RudiRSSClient::RudiRSSClient() : m_dbSemaphore{ nullptr }, m_dbConsumptionThread{ nullptr },
m_runDbConsumption{ FALSE }, m_keepNotification{ FALSE }, m_dbNotificationThread{ nullptr }
{
    m_dbLock.Init();
    m_dbSemaphore = CreateSemaphore(nullptr, 0, DEFAULT_MAX_CONSUMPTION_COUNT, nullptr);

    m_notificationLock.Init();
    m_notificationSemaphore = CreateSemaphore(nullptr, 0, DEFAULT_MAX_CONSUMPTION_COUNT, nullptr);

    WCHAR appDataDir[MAX_PATH]{};
    SHGetSpecialFolderPath(NULL, appDataDir, CSIDL_APPDATA, FALSE);
    m_rudirssDirectory = std::wstring(appDataDir) + L"\\rudirss";
    CreateDirectory(m_rudirssDirectory.c_str(), nullptr);
    m_rudirssIni = m_rudirssDirectory + L"\\rudirss.ini";
    m_rudirssDbPath = m_rudirssDirectory + L"\\rudirss.db";
}

RudiRSSClient::~RudiRSSClient()
{
    m_refreshFeedTimer.Delete();
    StopDBConsumption();

    CloseHandle(m_dbSemaphore);
    CloseHandle(m_notificationSemaphore);
}

bool RudiRSSClient::Initialize()
{
    try
    {
        m_db.Open(m_rudirssDbPath);
        m_db.Initialize();
    }
    catch (const std::exception& e)
    {
        return false;
    }

    StartDBConsumption();

    return FeedClient::Initialize();
}

void RudiRSSClient::OnFeedReady(const std::unique_ptr<Feed>& feed)
{
    if (feed)
        PushDBConsumptionUnit(feed);

    NotifyDbInsertionComplete(feed);
}

unsigned __stdcall RudiRSSClient::ThreadDBConsumption(void* param)
{
    auto pThis = reinterpret_cast<RudiRSSClient*>(param);
    pThis->OnDBConsumption();
    return 0;
}

unsigned __stdcall RudiRSSClient::ThreadDBNotification(void* param)
{
    auto pThis = reinterpret_cast<RudiRSSClient*>(param);
    pThis->OnDBNotification();
    return 0;
}

void RudiRSSClient::OnDBConsumption()
{
    while (WAIT_OBJECT_0 == WaitForSingleObject(m_dbSemaphore, INFINITE))
    {
        if (!InterlockedOr(reinterpret_cast<LONG*>(&m_runDbConsumption), 0))
            break;
        
        FeedDatabase::FeedConsumptionUnit consumptionUnit;
        if (!PopDBConsumptionUnit(consumptionUnit))
            continue;

        if (FeedDatabase::FeedConsumptionUnit::OperationType::INSERT_DATA == consumptionUnit.opType)
        {
            m_db.InsertFeed(consumptionUnit.feed);
            for (const auto& feedData : consumptionUnit.feedDataContainer)
            {
                m_db.InsertFeedData(feedData);
            }
        }
        else
        {
            {
                ATL::CComCritSecLock lock(m_notificationLock);
                m_notificationQueue.push(std::move(consumptionUnit));
            }
            ::ReleaseSemaphore(m_notificationSemaphore, 1, nullptr);
        }
    }
}

void RudiRSSClient::OnDBNotification()
{
    while (WAIT_OBJECT_0 == WaitForSingleObject(m_notificationSemaphore, INFINITE))
    {
        if (!InterlockedOr(reinterpret_cast<LONG*>(&m_keepNotification), 0))
            break;
        
        ATL::CComCritSecLock lock(m_notificationLock);
        if (!m_notificationQueue.empty())
        {
            auto consumptionUnit = std::move(m_notificationQueue.front());
            m_notificationQueue.pop();
            if (m_fnOnDbNotification)
                m_fnOnDbNotification(consumptionUnit);
        }
    }
}

void RudiRSSClient::StartDBConsumption()
{
    StopDBConsumption();

    if (!m_dbNotificationThread)
    {
        m_keepNotification = TRUE;
        m_dbNotificationThread = (HANDLE)_beginthreadex(nullptr, 0, ThreadDBNotification, this, 0, nullptr);
    }

    if (!m_dbConsumptionThread)
    {
        m_runDbConsumption = TRUE;
        m_dbConsumptionThread = (HANDLE)_beginthreadex(nullptr, 0, ThreadDBConsumption, this, 0, nullptr);
    }
}

void RudiRSSClient::StopDBConsumption()
{
    if (m_dbNotificationThread)
    {
        InterlockedExchange(reinterpret_cast<unsigned long*>(&m_keepNotification), FALSE);
        ::ReleaseSemaphore(m_notificationSemaphore, 1, nullptr);
        WaitForSingleObject(m_dbNotificationThread, INFINITE);
        CloseHandle(m_dbNotificationThread);
        m_dbNotificationThread = nullptr;
    }

    if (m_dbConsumptionThread)
    {
        InterlockedExchange(reinterpret_cast<unsigned long*>(&m_runDbConsumption), FALSE);
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
    consumptionUnit.opType = FeedDatabase::FeedConsumptionUnit::OperationType::INSERT_DATA;
    FeedCommon::ConvertWideStringToString(feedBase->GetFeedUrl(), consumptionUnit.feed.guid);
    consumptionUnit.feed.url = consumptionUnit.feed.guid;
    FeedCommon::ConvertWideStringToString(feed->GetValue(L"title"), consumptionUnit.feed.title);

    feed->IterateFeeds([&](const FeedData& feedData) -> bool {
        FeedDatabase::FeedData dbFeedData;
        FeedCommon::ConvertWideStringToString(feedData.GetValue(FeedCommon::FeedSpecification::RSS == spec ? L"link" : L"id"), dbFeedData.guid);
        dbFeedData.feedguid = consumptionUnit.feed.guid;
        dbFeedData.link = dbFeedData.guid;
        FeedCommon::ConvertWideStringToString(feedData.GetValue(L"title"), dbFeedData.title);
        FeedCommon::ConvertWideStringToString(feedData.GetValue(FeedCommon::FeedSpecification::RSS == spec ? L"pubDate" : L"updated"), dbFeedData.datetime);
        dbFeedData.timestamp = FeedCommon::ConvertDatetimeToTimestamp(spec, dbFeedData.datetime);
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

void RudiRSSClient::NotifyDbInsertionComplete(const std::unique_ptr<Feed>& feed)
{
    FeedDatabase::FeedConsumptionUnit consumptionUnit;
    consumptionUnit.opType = FeedDatabase::FeedConsumptionUnit::OperationType::NOTIFY_INSERTION_COMPLETE;
    FeedBase* feedBase = reinterpret_cast<FeedBase*>(feed.get());
    FeedCommon::ConvertWideStringToString(feedBase->GetFeedUrl(), consumptionUnit.feed.guid);

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

bool RudiRSSClient::QueryFeed(const std::string& guid, FeedDatabase::FN_QUERY_FEED fnQueryFeed)
{
    return m_db.QueryFeed(guid, fnQueryFeed);
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

void RudiRSSClient::StartRefreshFeedTimer(DWORD dueTime, DWORD period, FN_ON_DB_NOTIFICATION fnOnDbNotification)
{
    m_fnOnDbNotification = fnOnDbNotification;
    m_refreshFeedTimer.Create([](PVOID param, BOOLEAN timerOrWaitFired) {
        RudiRSSClient* rudiRSSClient = reinterpret_cast<RudiRSSClient*>(param);
        RudiRSSClient::Configuration config;
        if (rudiRSSClient->LoadConfig(config))
        {
            for (const auto& feedUrl : config.feedUrls)
            {
                rudiRSSClient->ConsumeFeed(feedUrl);
            }
        }

        }, this, dueTime, period, WT_EXECUTEDEFAULT);
}

bool RudiRSSClient::LoadConfig(Configuration& config)
{
    config.feedUrls.clear();
    std::vector<WCHAR> data(2048, 0);
    int feedCount = GetPrivateProfileInt(L"Feed", L"Count", 0, m_rudirssIni.c_str());
    for (int c = 0; c < feedCount; c++)
    {
        DWORD size = GetPrivateProfileString(L"Feed", std::format(L"Feed_{}", c).c_str(), L"", data.data(), data.size(), m_rudirssIni.c_str());
        config.feedUrls.push_back(std::wstring(data.data(), size));
    }

    return !config.feedUrls.empty();
}

