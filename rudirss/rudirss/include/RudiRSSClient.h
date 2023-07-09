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

    using FN_ON_REFRESH_FEED_COMPLETE = std::function<void(const FeedDatabase::Feed& feed)>;
    virtual bool Initialize();
    bool QueryFeed(long long feedId, FeedDatabase::FN_QUERY_FEED fnQueryFeed);
    bool QueryAllFeeds(FeedDatabase::FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedData(const std::string& guid, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedData(long long feeddataid, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData);

    using FN_ON_FEES_REFRESH_COMPLETE = std::function<void()>;
    void StartRefreshFeedTimer(DWORD dueTime, DWORD period);

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
    Timer m_refreshFeedTimer;
    std::wstring m_rudirssDirectory;
    std::wstring m_rudirssIni;
    std::wstring m_rudirssDbPath;

    virtual void OnFeedReady(const std::unique_ptr<Feed>& feed);

    static unsigned __stdcall ThreadDBConsumption(void* param);
    void StartDBConsumption();
    void StopDBConsumption();
    void PushDBConsumptionUnit(const std::unique_ptr<Feed>& feed);
    void PushDBNotifyInsertionCompleteEvent();
    bool PopDBConsumptionUnit(FeedDatabase::FeedConsumptionUnit &consumptionUnit);
    void DBConsumption();
};
