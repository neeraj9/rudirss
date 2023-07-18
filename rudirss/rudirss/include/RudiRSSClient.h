#pragma once

#include "FeedClient.h"
#include "FeedCommon.h"
#include "FeedDatabase.h"
#include "Timer.h"
#include "RefreshTimer.h"
#include "Configuration.h"

#include <functional>
#include <queue>
#include <map>

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
    bool QueryFeedDataByOffsetOrderByTimestamp(long long offset, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedDataByFeedIdByOffsetOrderByTimestamp(long long feedid, long long offset, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryAllFeedDataOrderByTimestamp(FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedDataByFeedDataId(long long feeddataid, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData);
    bool DeleteOutdatedFeedData(unsigned reserveDays);
    bool DeleteFeedByFeedId(long long feedid);
    bool DeleteFeedDataByFeedId(long long feedid);
    bool UpdateFeedDataReadColumn(long long feeddataid, long long read);
    bool QueryFeedTableDataExist(long long& exitst);
    bool QueryFeedDataTableCount(long long &count);
    bool QueryFeedDataTableCountByFeedId(long long feedid, long long &count);
    bool QueryFeedDataByOffset(long long offset, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedTableCount(long long &count);
    bool QueryFeedByOffset(long long offset, FeedDatabase::FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedByOffsetInRange(long long limit, long long offset, FeedDatabase::FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedDataOrderByTimestampInRange(long long limit, long long offset, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedDataByFeedIdOrderByTimestampInRange(long long feedid, long long limit, long long offset, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedExistByGuid(const std::string& guid, long long& exist);

    using FN_ON_DB_NOTIFICATION = std::function<void(const FeedDatabase::FeedConsumptionUnit &)>;
    void StartRefreshFeedTimer(FN_ON_DB_NOTIFICATION fnOnDbNotification);
    using FN_ON_IMPORT_OPML = std::function<void(const std::vector<std::wstring>&)>;
    void ImportFromOPML(const std::wstring &opml, FN_ON_IMPORT_OPML fnOnImportOPML);
    using FN_ON_IMPORT_LIST_FILE = std::function<void(const std::vector<std::wstring>&)>;
    void ImportFromListFile(const std::wstring &listFile, FN_ON_IMPORT_LIST_FILE fnOnImportListFile);

    void LoadDisplayConfiguration(DisplayConfiguration& displayConfig);
    void SaveDisplayConfiguration(const DisplayConfiguration& displayConfig);

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

    std::map<std::wstring, RefreshTimer> m_refreshTimer;
    ATL::CComCriticalSection m_refreshTimerLock;
    std::wstring m_rudirssDirectory;
    std::wstring m_rudirssIni;
    std::wstring m_rudirssDbPath;

    FN_ON_DB_NOTIFICATION m_fnOnDbNotification;

    void LoadDatabaseConfiguration(DatabaseConfiguration& dbConfig);
    void SaveDatabaseConfiguration(const DatabaseConfiguration& dbConfig);

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
