#include "RudiRSSClient.h"
#include "FeedBase.h"
#include "FeedCommon.h"

#include <userenv.h>
#include <shlobj_core.h>
#include <format>
#include <atltime.h>

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

        DatabaseConfiguration dbConfig;
        LoadDatabaseConfiguration(dbConfig);
        if (dbConfig.allowDeleteOutdatedFeedItems)
            DeleteOutdatedFeedData(dbConfig.reserveDays);
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
            m_db.QueryFeedByGuid(consumptionUnit.feed.guid, [&](const FeedDatabase::Feed& feed) {
                consumptionUnit.feed.feedid = feed.feedid;
                });
            for (auto& feedData : consumptionUnit.feedDataContainer)
            {
                feedData.feedid = consumptionUnit.feed.feedid;
                m_db.InsertFeedData(feedData);
            }

            if (consumptionUnit.allowInsertionNotification)
            {
                consumptionUnit.opType = FeedDatabase::FeedConsumptionUnit::OperationType::NOTIFY_INSERTION_COMPLETE;
                NotifyDbEvent(std::move(consumptionUnit));
            }
        }
        else
        {
            NotifyDbEvent(std::move(consumptionUnit));
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
    m_refreshTimer.clear();

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
        FeedCommon::ConvertWideStringToString(feedData.GetValue(L"link"), dbFeedData.guid);
        dbFeedData.feedid = consumptionUnit.feed.feedid;
        dbFeedData.link = dbFeedData.guid;
        FeedCommon::ConvertWideStringToString(feedData.GetValue(L"title"), dbFeedData.title);
        FeedCommon::ConvertWideStringToString(feedData.GetValue(FeedCommon::FeedSpecification::RSS == spec ? L"pubDate" : L"updated"), dbFeedData.datetime);
        dbFeedData.timestamp = FeedCommon::ConvertDatetimeToTimestamp(spec, dbFeedData.datetime);
        dbFeedData.createdtime = time(nullptr);
        consumptionUnit.feedDataContainer.push_back(std::move(dbFeedData));
        return true;
        });

    {
        ATL::CComCritSecLock lock(m_dbLock);
        m_dbQueue.push(std::move(consumptionUnit));
    }
    ::ReleaseSemaphore(m_dbSemaphore, 1, nullptr);
}

void RudiRSSClient::NotifyDbEvent(FeedDatabase::FeedConsumptionUnit&& consumptionUnit)
{
    {
        ATL::CComCritSecLock lock(m_notificationLock);
        m_notificationQueue.push(std::move(consumptionUnit));
    }
    ::ReleaseSemaphore(m_notificationSemaphore, 1, nullptr);
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

bool RudiRSSClient::QueryFeedByGuid(const std::string& guid, FeedDatabase::FN_QUERY_FEED fnQueryFeed)
{
    return m_db.QueryFeedByGuid(guid, fnQueryFeed);
}

bool RudiRSSClient::QueryAllFeeds(FeedDatabase::FN_QUERY_FEED fnQueryFeed)
{
    return m_db.QueryAllFeeds(fnQueryFeed);
}

bool RudiRSSClient::QueryFeedDataByFeedId(long long feedid, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData)
{
    return m_db.QueryFeedDataByFeedId(feedid, fnQueryFeedData);
}

bool RudiRSSClient::QueryFeedDataOrderByTimestamp(long long feedid, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData)
{
    return m_db.QueryFeedDataOrderByTimestamp(feedid, fnQueryFeedData);
}

bool RudiRSSClient::QueryFeedDataByOffsetOrderByTimestamp(long long offset, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData)
{
    return m_db.QueryFeedDataByOffsetOrderByTimestamp(offset, fnQueryFeedData);
}

bool RudiRSSClient::QueryFeedDataByFeedIdByOffsetOrderByTimestamp(long long feedid, long long offset, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData)
{
    return m_db.QueryFeedDataByFeedIdByOffsetOrderByTimestamp(feedid, offset, fnQueryFeedData);
}

bool RudiRSSClient::QueryAllFeedDataOrderByTimestamp(FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData)
{
    return m_db.QueryAllFeedDataOrderByTimestamp(fnQueryFeedData);
}

bool RudiRSSClient::QueryFeedDataByFeedDataId(long long feeddataid, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData)
{
    return m_db.QueryFeedDataByFeedDataId(feeddataid, fnQueryFeedData);
}

bool RudiRSSClient::DeleteOutdatedFeedData(unsigned reserveDays)
{
    CTime current(time(nullptr));
    CTime today(current.GetYear(), current.GetMonth(), current.GetDay(), 0, 0, 0);
    CTimeSpan span(reserveDays, 0, 0, 0);
    today -= span;
    return m_db.DeleteOutdatedFeedData(today.GetTime());
}

bool RudiRSSClient::DeleteFeedByFeedId(long long feedid)
{
    return m_db.DeleteFeedByFeedId(feedid);
}

bool RudiRSSClient::DeleteFeedDataByFeedId(long long feedid)
{
    return m_db.DeleteFeedDataByFeedId(feedid);
}

bool RudiRSSClient::UpdateFeedDataReadColumn(long long feeddataid, long long read)
{
    return m_db.UpdateFeedDataReadColumn(feeddataid, read);
}

bool RudiRSSClient::QueryFeedTableDataExist(long long& exist)
{
    return m_db.QueryFeedTableDataExist(exist);
}

bool RudiRSSClient::QueryFeedDataTableCount(long long& count)
{
    return m_db.QueryFeedDataTableCount(count);
}

bool RudiRSSClient::QueryFeedDataTableCountByFeedId(long long feedid, long long& count)
{
    return m_db.QueryFeedDataTableCountByFeedId(feedid, count);
}

bool RudiRSSClient::QueryFeedDataByOffset(long long offset, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData)
{
    return m_db.QueryFeedDataByOffset(offset, fnQueryFeedData);
}

bool RudiRSSClient::QueryFeedTableCount(long long& count)
{
    return m_db.QueryFeedTableCount(count);
}

bool RudiRSSClient::QueryFeedByOffset(long long offset, FeedDatabase::FN_QUERY_FEED fnQueryFeed)
{
    return m_db.QueryFeedByOffset(offset, fnQueryFeed);
}

VOID CALLBACK RudiRSSClient::WaitOrTimerCallback(PVOID param, BOOLEAN TimerOrWaitFired)
{
    auto refreshTimer = reinterpret_cast<RefreshTimer*>(param);
    auto pThis = reinterpret_cast<RudiRSSClient*>(refreshTimer->GetParam());
    // By design, guid is identical to url in Feed table
    pThis->ConsumeFeed(refreshTimer->GetFeedGuid());
}

void RudiRSSClient::StartRefreshFeedTimer(FN_ON_DB_NOTIFICATION fnOnDbNotification)
{
    m_fnOnDbNotification = fnOnDbNotification;

    long long feedTableHasData = 0;
    if (QueryFeedTableDataExist(feedTableHasData)
        && 1 == feedTableHasData)
    {
        QueryAllFeeds([&](const FeedDatabase::Feed& feed) {
            std::wstring guid;
            FeedCommon::ConvertStringToWideString(feed.guid, guid);
            auto pair = m_refreshTimer.insert(std::pair<std::wstring, RefreshTimer>(std::move(guid), std::move(RefreshTimer(this, guid))));
            if (pair.second)
            {
                pair.first->second.Create(WaitOrTimerCallback, &pair.first->second, feed.duetime, feed.updateinterval, WT_EXECUTEDEFAULT);
            }
            });
    }
}

void RudiRSSClient::ImportFromOPML(const std::wstring& opml, FN_ON_IMPORT_OPML fnOnImportOPML)
{
    std::vector<std::wstring> feedUrls;
    if (FeedCommon::LoadFeedUrlsFromOPML(opml, feedUrls))
    {
        for (const auto& feedUrl : feedUrls)
        {
            auto it = m_refreshTimer.find(feedUrl);
            if (it == m_refreshTimer.end())
            {
                auto pair = m_refreshTimer.insert(std::pair<std::wstring, RefreshTimer>(feedUrl, std::move(RefreshTimer(this, feedUrl))));
                if (pair.second)
                {
                    pair.first->second.Create(WaitOrTimerCallback, &pair.first->second, FeedDatabase::Feed::DEFAULT_FEED_UPDATE_DUETIME,
                        FeedDatabase::Feed::DEFAULT_FEED_UPDATE_INTERVAL, WT_EXECUTEDEFAULT);
                }
            }
            else
            {
                it->second.Create(WaitOrTimerCallback, &it->second, FeedDatabase::Feed::DEFAULT_FEED_UPDATE_DUETIME,
                    FeedDatabase::Feed::DEFAULT_FEED_UPDATE_INTERVAL, WT_EXECUTEDEFAULT);
            }
        }

        if (fnOnImportOPML)
            fnOnImportOPML(feedUrls);
    }
}

void RudiRSSClient::LoadDatabaseConfiguration(DatabaseConfiguration& dbConfig)
{
    dbConfig.allowDeleteOutdatedFeedItems = !!GetPrivateProfileInt(L"Database", L"AllowDeleteOutdatedFeedItems", 1, m_rudirssIni.c_str());
    dbConfig.reserveDays = GetPrivateProfileInt(L"Database", L"ReserveDays", DatabaseConfiguration::DEFAULT_RESERVE_DAYS, m_rudirssIni.c_str());
}

void RudiRSSClient::SaveDatabaseConfiguration(const DatabaseConfiguration& dbConfig)
{
    WritePrivateProfileString(L"Database", L"AllowDeleteOutdatedFeedItems",
        std::to_wstring(static_cast<unsigned>(dbConfig.allowDeleteOutdatedFeedItems)).c_str(), m_rudirssIni.c_str());
    WritePrivateProfileString(L"Database", L"ReserveDays", std::to_wstring(dbConfig.reserveDays).c_str(), m_rudirssIni.c_str());
}

void RudiRSSClient::LoadDisplayConfiguration(DisplayConfiguration &displayConfig)
{
    displayConfig.feedWidth = GetPrivateProfileInt(L"Display", L"FeedWidth", 300, m_rudirssIni.c_str());
    displayConfig.feedItemTitleColumnWidth = GetPrivateProfileInt(L"Display", L"FeedItemTitleColumnWidth", 250, m_rudirssIni.c_str());
    displayConfig.feedItemUpdatedColumnWidth = GetPrivateProfileInt(L"Display", L"FeedItemUpdatedColumnWidth", 150, m_rudirssIni.c_str());
}

void RudiRSSClient::SaveDisplayConfiguration(const DisplayConfiguration &displayConfig)
{
    WritePrivateProfileString(L"Display", L"FeedWidth",
        std::to_wstring(static_cast<unsigned>(displayConfig.feedWidth)).c_str(), m_rudirssIni.c_str());
    WritePrivateProfileString(L"Display", L"FeedItemTitleColumnWidth",
        std::to_wstring(static_cast<unsigned>(displayConfig.feedItemTitleColumnWidth)).c_str(), m_rudirssIni.c_str());
    WritePrivateProfileString(L"Display", L"FeedItemUpdatedColumnWidth",
        std::to_wstring(static_cast<unsigned>(displayConfig.feedItemUpdatedColumnWidth)).c_str(), m_rudirssIni.c_str());
}
