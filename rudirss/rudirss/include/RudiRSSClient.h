#pragma once

#include "FeedClient.h"
#include "FeedCommon.h"
#include "FeedDatabase.h"
#include "Timer.h"
#include "Configuration.h"

#include <functional>
#include <vector>
#include <queue>

class RudiRSSClient : public FeedClient
{
public:
    RudiRSSClient();
    virtual ~RudiRSSClient();

    virtual bool Initialize();
    bool QueryFeed(long long feedId, FeedDatabase::FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedByGuid(const std::string &guid, FeedDatabase::FN_QUERY_FEED fnQueryFeed);
    bool QueryAllFeeds(FeedDatabase::FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedDataByFeedId(long long feedid, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedDataOrderByTimestamp(long long feedid, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryAllFeedDataOrderByTimestamp(FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedDataByFeedDataId(long long feeddataid, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData);
    bool DeleteOutdatedFeedData(unsigned reserveDays);
    bool UpdateFeedDataReadColumn(long long feeddataid, long long read);

    using FN_ON_DB_NOTIFICATION = std::function<void(const FeedDatabase::FeedConsumptionUnit &)>;
    using FN_ON_PREPARE_REFRESH_FEED = std::function<void(const Configuration& congiguration)>;
    void StartRefreshFeedTimer(FN_ON_PREPARE_REFRESH_FEED fnOnPrepareRefreshFeed, FN_ON_DB_NOTIFICATION fnOnDbNotification);

protected:
    static const size_t DEFAULT_MAX_CONSUMPTION_COUNT = 32768;

    FeedDatabase m_db;
    ATL::CComCriticalSection m_dbLock;
    HANDLE m_dbSemaphore;
    HANDLE m_dbConsumptionThread;
    BOOL m_runDbConsumption;
    std::queue<FeedDatabase::FeedConsumptionUnit> m_dbQueue;

    HANDLE m_notificationSemaphore;
    HANDLE m_dbNotificationThread;
    BOOL m_keepNotification;
    ATL::CComCriticalSection m_notificationLock;
    std::queue<FeedDatabase::FeedConsumptionUnit> m_notificationQueue;

    Timer m_refreshFeedTimer;
    std::wstring m_rudirssDirectory;
    std::wstring m_rudirssIni;
    std::wstring m_rudirssDbPath;

    FN_ON_PREPARE_REFRESH_FEED m_fnOnPrepareRefreshFeed;
    FN_ON_DB_NOTIFICATION m_fnOnDbNotification;

    Configuration m_lastLoadedConfig;
    ATL::CComCriticalSection m_configurationLock;

    void LoadTimerConfiguration(TimerConfiguration &timerConfig);
    void LoadDatabaseConfiguration(DatabaseConfiguration& dbConfig);
    bool LoadConfiguration(Configuration &config);
    void SaveTimerConfiguration(TimerConfiguration &timerConfig);
    void SaveDatabaseConfiguration(DatabaseConfiguration& dbConfig);
    void SaveConfiguration(Configuration &config);

    virtual void OnFeedReady(const std::unique_ptr<Feed>& feed);
    void NotifyDbEvent(FeedDatabase::FeedConsumptionUnit &&consumptionUnit);

    static unsigned __stdcall ThreadDBConsumption(void* param);
    static unsigned __stdcall ThreadDBNotification(void* param);
    void StartDBConsumption();
    void StopDBConsumption();
    void PushDBConsumptionUnit(const std::unique_ptr<Feed>& feed);
    bool PopDBConsumptionUnit(FeedDatabase::FeedConsumptionUnit &consumptionUnit);
    void OnDBConsumption();
    void OnDBNotification();

    static VOID CALLBACK WaitOrTimerCallback(PVOID param, BOOLEAN TimerOrWaitFired);
};
