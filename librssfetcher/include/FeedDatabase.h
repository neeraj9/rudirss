#pragma once

#include "SQLite3Handle.h"
#include "SQLite3StmtHandle.h"

#include <string>
#include <time.h>
#include <functional>
#include <atlcore.h>

class FeedDatabase
{
protected:
    SQLite3Handle m_sql;
    SQLite3StmtHandle m_insertFeedStmt;
    SQLite3StmtHandle m_insertFeedDataStmt;
    SQLite3StmtHandle m_queryFeedStmt;
    SQLite3StmtHandle m_queryFeedByGuidStmt;
    SQLite3StmtHandle m_queryAllFeedsStmt;
    SQLite3StmtHandle m_queryFeedDataByGuidStmt;
    SQLite3StmtHandle m_queryFeedDataByFeedDataIdStmt;
    SQLite3StmtHandle m_queryFeedDataByGuidOrderByTimestampStmt;
    SQLite3StmtHandle m_queryAllFeedDataOrderByTimestampStmt;
    SQLite3StmtHandle m_deleteAllFeedStmt;
    SQLite3StmtHandle m_deleteAllFeedDataStmt;
    ATL::CComCriticalSection m_dbLock;

public:
    FeedDatabase();
    virtual ~FeedDatabase();

    bool Open(const std::wstring &dbPath);
    void Initialize();
    void Close();

    struct Feed
    {
        long long feedid;
        std::string guid;
        std::string url;
        std::string title;
        Feed() : feedid{ 0 } {}
        Feed(const Feed& rhs) : feedid{ rhs.feedid }, guid{ rhs.guid }, url{ rhs.url }, title{ rhs.title } {}
        Feed(Feed&& rhs) noexcept : feedid{ rhs.feedid }, guid{ std::move(rhs.guid) }, url{ std::move(rhs.url) }, title{ rhs.title } {}
        Feed& operator=(Feed&& rhs) noexcept
        {
            if (this != &rhs)
            {
                feedid = rhs.feedid;
                guid = std::move(rhs.guid);
                url = std::move(rhs.url);
                title = std::move(rhs.title);
            }

            return *this;
        }
    };

    static const long long INVALID_FEEDDATA_ID = -1;
    struct FeedData
    {
        long long feeddataid;
        std::string guid;
        std::string feedguid;
        std::string link;
        std::string title;
        std::string datetime;
        time_t timestamp;
        time_t createdtime;
        std::string tag;
        std::string misc;
        FeedData() : feeddataid{ INVALID_FEEDDATA_ID }, timestamp{ 0 }, createdtime{ 0 } {}
        FeedData(const FeedData& rhs) : feeddataid{ rhs.feeddataid }, guid{ rhs.guid }, feedguid{ rhs.feedguid }, link{ rhs.link }, title{ rhs.title },
            datetime{ rhs.datetime }, timestamp{ rhs.timestamp }, createdtime{ rhs.createdtime }, tag{ rhs.tag },
            misc{ rhs.misc } {}
        FeedData(FeedData&& rhs) noexcept : feeddataid{ rhs.feeddataid }, guid{ std::move(rhs.guid) }, feedguid{ std::move(rhs.feedguid) },
            link{ std::move(rhs.link) }, title{ std::move(rhs.title) }, datetime{ std::move(rhs.datetime) }, timestamp{ rhs.timestamp },
            createdtime{ rhs.createdtime }, tag{ std::move(rhs.tag) }, misc{ std::move(rhs.misc) } {}
        FeedData& operator=(FeedData&& rhs) noexcept
        {
            if (this != &rhs)
            {
                feeddataid = rhs.feeddataid;
                guid = std::move(rhs.guid);
                feedguid = std::move(rhs.feedguid);
                link = std::move(rhs.link);
                title = std::move(rhs.title);
                datetime = std::move(rhs.datetime);
                timestamp = rhs.timestamp;
                createdtime = rhs.createdtime;
                tag = std::move(rhs.tag);
                misc = std::move(rhs.misc);
            }

            return *this;
        }
    };

    struct FeedConsumptionUnit
    {
        enum class OperationType
        {
            INSERT_DATA,
            NOTIFY_INSERTION_COMPLETE,
        };

        OperationType opType;
        Feed feed;
        std::vector<FeedData> feedDataContainer;
        FeedConsumptionUnit() : opType{ OperationType::INSERT_DATA } {}
        FeedConsumptionUnit(const FeedConsumptionUnit& rhs) :opType{ rhs.opType }, feed{ rhs.feed },
            feedDataContainer{ rhs.feedDataContainer } {}
        FeedConsumptionUnit(FeedConsumptionUnit&& rhs) noexcept :opType{ rhs.opType }, feed{ std::move(rhs.feed) },
            feedDataContainer{ std::move(rhs.feedDataContainer) } {}
        FeedConsumptionUnit& operator=(FeedConsumptionUnit&& rhs) noexcept
        {
            if (this != &rhs)
            {
                opType = rhs.opType;
                feed = std::move(rhs.feed);
                feedDataContainer = std::move(rhs.feedDataContainer);
            }

            return *this;
        }
    };

    bool InsertFeed(const Feed& feed);
    bool InsertFeedData(const FeedData& feedData);

    using FN_QUERY_FEED = std::function<void(const Feed&)>;
    using FN_QUERY_FEED_DATA = std::function<void(const FeedData&)>;
    bool QueryFeed(long long feedId, FN_QUERY_FEED fnQueryFeed);
    bool QueryFeed(const std::string &guid, FN_QUERY_FEED fnQueryFeed);
    bool QueryAllFeeds(FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedData(const std::string& guid, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedDataOrderByTimestamp(const std::string& guid, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryAllFeedDataOrderByTimestamp(FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedData(long long feeddataid, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool DeleteAllFeeds();
    bool DeleteAllFeedData();
};
