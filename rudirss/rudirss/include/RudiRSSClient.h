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
    bool QueryAllFeeds(FeedDatabase::FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedData(const std::string& guid, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedData(long long feeddataid, FeedDatabase::FN_QUERY_FEED_DATA fnQueryFeedData);

    using FN_ON_REFRESH_FEEDS_COMPLETE = std::function<void()>;
    struct TimerParameter
    {
        FN_ON_REFRESH_FEEDS_COMPLETE fnOnRefreshFeedsComplete;
        RudiRSSClient* rudiRSSClient;
    };
    bool InitializeRefreshFeedTimer(FN_ON_REFRESH_FEEDS_COMPLETE fnOnRefreshFeedsComplete, DWORD dueTime, DWORD period);

protected:
    static const size_t DEFAULT_MAX_CONSUMPTION_COUNT = 32768;
    FeedDatabase m_db;
    ATL::CComCriticalSection m_dbLock;
    HANDLE m_dbSemaphore;
    HANDLE m_dbStopEvent;
    HANDLE m_dbConsumptionThread;
    std::queue<FeedDatabase::FeedConsumptionUnit> m_dbQueue;
    Timer m_refreshFeedTimer;
    TimerParameter m_timerParam;

    virtual void OnFeedReady(const std::unique_ptr<Feed>& feed);

    static unsigned __stdcall ThreadDBConsumption(void* param);
    void StartDBConsumption();
    void StopDBConsumption();
    void PushDBConsumptionUnit(const std::unique_ptr<Feed>& feed);
    bool PopDBConsumptionUnit(FeedDatabase::FeedConsumptionUnit &consumptionUnit);
    void DBConsumption();
};
