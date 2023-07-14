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

    m_configurationLock.Init();

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

    {
        ATL::CComCritSecLock lock(m_configurationLock);
        m_lastLoadedConfig.feedUrls.clear();
        QueryAllFeeds([&](const FeedDatabase::Feed& feed) {
            std::wstring url;
            FeedCommon::ConvertStringToWideString(feed.url, url);
            m_lastLoadedConfig.feedUrls.push_back(url);
            });
        SaveConfiguration(m_lastLoadedConfig);
    }
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

bool RudiRSSClient::UpdateFeedDataReadColumn(long long feeddataid, long long read)
{
    return m_db.UpdateFeedDataReadColumn(feeddataid, read);
}

VOID CALLBACK RudiRSSClient::WaitOrTimerCallback(PVOID param, BOOLEAN TimerOrWaitFired)
{
    RudiRSSClient* pThis = reinterpret_cast<RudiRSSClient*>(param);
    Configuration config;
    if (pThis->LoadConfiguration(config))
    {
        {
            ATL::CComCritSecLock lock(pThis->m_configurationLock);
            pThis->m_lastLoadedConfig = config;
        }

        for (const auto& feedUrl : config.feedUrls)
        {
            pThis->ConsumeFeed(feedUrl);
        }
    }
}

void RudiRSSClient::StartRefreshFeedTimer(FN_ON_DB_NOTIFICATION fnOnDbNotification)
{
    m_fnOnDbNotification = fnOnDbNotification;

    TimerConfiguration timerConfig;
    LoadTimerConfiguration(timerConfig);
    m_refreshFeedTimer.Create(WaitOrTimerCallback, this, timerConfig.dueTime, timerConfig.period, WT_EXECUTEDEFAULT);
}

void RudiRSSClient::LoadTimerConfiguration(TimerConfiguration& timerConfig)
{
    timerConfig.dueTime = GetPrivateProfileInt(L"Timer", L"DueTime", TimerConfiguration::DEFAULT_DUETIME, m_rudirssIni.c_str());
    timerConfig.period = GetPrivateProfileInt(L"Timer", L"Period", TimerConfiguration::DEFAULT_PERIOD, m_rudirssIni.c_str());
}

void RudiRSSClient::LoadDatabaseConfiguration(DatabaseConfiguration& dbConfig)
{
    dbConfig.allowDeleteOutdatedFeedItems = !!GetPrivateProfileInt(L"Database", L"AllowDeleteOutdatedFeedItems", 1, m_rudirssIni.c_str());
    dbConfig.reserveDays = GetPrivateProfileInt(L"Database", L"ReserveDays", DatabaseConfiguration::DEFAULT_RESERVE_DAYS, m_rudirssIni.c_str());
}

bool RudiRSSClient::LoadConfiguration(Configuration& config)
{
    LoadTimerConfiguration(config.timerConfiguration);
    LoadDatabaseConfiguration(config.dbConfiguration);

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

void RudiRSSClient::SaveTimerConfiguration(TimerConfiguration& timerConfig)
{
    WritePrivateProfileString(L"Timer", L"DueTime", std::to_wstring(timerConfig.dueTime).c_str(), m_rudirssIni.c_str());
    WritePrivateProfileString(L"Timer", L"Period", std::to_wstring(timerConfig.period).c_str(), m_rudirssIni.c_str());
}

void RudiRSSClient::SaveDatabaseConfiguration(DatabaseConfiguration& dbConfig)
{
    WritePrivateProfileString(L"Database", L"AllowDeleteOutdatedFeedItems",
        std::to_wstring(static_cast<unsigned>(dbConfig.allowDeleteOutdatedFeedItems)).c_str(), m_rudirssIni.c_str());
    WritePrivateProfileString(L"Database", L"ReserveDays", std::to_wstring(dbConfig.reserveDays).c_str(), m_rudirssIni.c_str());
}

void RudiRSSClient::SaveConfiguration(Configuration& config)
{
    SaveTimerConfiguration(config.timerConfiguration);
    SaveDatabaseConfiguration(config.dbConfiguration);

    WritePrivateProfileString(L"Feed", L"Count", std::to_wstring(config.feedUrls.size()).c_str(), m_rudirssIni.c_str());
    for (size_t c = 0; c < config.feedUrls.size(); c++)
    {
        WritePrivateProfileString(L"Feed", std::format(L"Feed_{}", c).c_str(), config.feedUrls[c].c_str(), m_rudirssIni.c_str());
    }
}

