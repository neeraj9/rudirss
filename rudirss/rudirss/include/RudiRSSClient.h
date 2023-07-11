#pragma once

#include "FeedClient.h"
#include "FeedCommon.h"
#include "FeedDatabase.h"
#include "Timer.h"

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
    bool UpdateFeedDataReadColumn(long long feeddataid, long long read);

    using FN_ON_DB_NOTIFICATION = std::function<void(const FeedDatabase::FeedConsumptionUnit &)>;
    void StartRefreshFeedTimer(DWORD dueTime, DWORD period, FN_ON_DB_NOTIFICATION fnOnDbNotification);

    struct Configuration
    {
        std::vector<std::wstring> feedUrls;
        Configuration() {}
        Configuration(const Configuration& rhs) : feedUrls{ rhs.feedUrls } {}
        Configuration(Configuration&& rhs) noexcept: feedUrls{ std::move(rhs.feedUrls) } {}
        Configuration& operator=(Configuration&& rhs) noexcept
        {
            if (this != &rhs)
            {
                feedUrls = std::move(rhs.feedUrls);
            }

            return *this;
        }
    };
    bool LoadConfig(Configuration &config);

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

    FN_ON_DB_NOTIFICATION m_fnOnDbNotification;

    virtual void OnFeedReady(const std::unique_ptr<Feed>& feed);
    void NotifyDbInsertionComplete(const std::unique_ptr<Feed>& feed);

    static unsigned __stdcall ThreadDBConsumption(void* param);
    static unsigned __stdcall ThreadDBNotification(void* param);
    void StartDBConsumption();
    void StopDBConsumption();
    void PushDBConsumptionUnit(const std::unique_ptr<Feed>& feed);
    bool PopDBConsumptionUnit(FeedDatabase::FeedConsumptionUnit &consumptionUnit);
    void OnDBConsumption();
    void OnDBNotification();
};
