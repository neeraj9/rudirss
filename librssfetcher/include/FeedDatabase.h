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
    SQLite3StmtHandle m_queryFeedDataByFeedIdStmt;
    SQLite3StmtHandle m_queryFeedDataByFeedDataIdStmt;
    SQLite3StmtHandle m_queryFeedDataByFeedIdOrderByTimestampStmt;
    SQLite3StmtHandle m_queryAllFeedDataOrderByTimestampStmt;
    SQLite3StmtHandle m_deleteAllFeedStmt;
    SQLite3StmtHandle m_deleteAllFeedDataStmt;
    SQLite3StmtHandle m_updateFeedDataReadStmt;
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
        long long feedid;
        std::string link;
        std::string title;
        std::string datetime;
        time_t timestamp;
        time_t createdtime;
        long long read;
        std::string tag;
        std::string misc;
        FeedData() : feeddataid{ INVALID_FEEDDATA_ID }, feedid{ 0 }, timestamp{ 0 }, createdtime{ 0 }, read{ static_cast<long long>(false) } {}
        FeedData(const FeedData& rhs) : feeddataid{ rhs.feeddataid }, guid{ rhs.guid }, feedid{ rhs.feedid }, link{ rhs.link },
            title{ rhs.title }, datetime{ rhs.datetime }, timestamp{ rhs.timestamp }, createdtime{ rhs.createdtime }, read{ rhs.read },
            tag{ rhs.tag }, misc{ rhs.misc } {}
        FeedData(FeedData&& rhs) noexcept : feeddataid{ rhs.feeddataid }, guid{ std::move(rhs.guid) }, feedid{ std::move(rhs.feedid) },
            link{ std::move(rhs.link) }, title{ std::move(rhs.title) }, datetime{ std::move(rhs.datetime) }, timestamp{ rhs.timestamp },
            createdtime{ rhs.createdtime }, read{ rhs.read }, tag{ std::move(rhs.tag) }, misc{ std::move(rhs.misc) } {}
        FeedData& operator=(FeedData&& rhs) noexcept
        {
            if (this != &rhs)
            {
                feeddataid = rhs.feeddataid;
                guid = std::move(rhs.guid);
                feedid = rhs.feedid;
                link = std::move(rhs.link);
                title = std::move(rhs.title);
                datetime = std::move(rhs.datetime);
                timestamp = rhs.timestamp;
                createdtime = rhs.createdtime;
                read = rhs.read;
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
        bool allowInsertionNotification;
        FeedConsumptionUnit() : opType{ OperationType::INSERT_DATA }, allowInsertionNotification{ true } {}
        FeedConsumptionUnit(const FeedConsumptionUnit& rhs) :opType{ rhs.opType }, feed{ rhs.feed },
            feedDataContainer{ rhs.feedDataContainer }, allowInsertionNotification{ rhs.allowInsertionNotification } {}
        FeedConsumptionUnit(FeedConsumptionUnit&& rhs) noexcept :opType{ rhs.opType }, feed{ std::move(rhs.feed) },
            feedDataContainer{ std::move(rhs.feedDataContainer) }, allowInsertionNotification{ rhs.allowInsertionNotification } {}
        FeedConsumptionUnit& operator=(FeedConsumptionUnit&& rhs) noexcept
        {
            if (this != &rhs)
            {
                opType = rhs.opType;
                feed = std::move(rhs.feed);
                feedDataContainer = std::move(rhs.feedDataContainer);
                allowInsertionNotification = rhs.allowInsertionNotification;
            }

            return *this;
        }
    };

    bool InsertFeed(const Feed& feed);
    bool InsertFeedData(const FeedData& feedData);
    sqlite3_int64 GetLastInsertRowid();

    using FN_QUERY_FEED = std::function<void(const Feed&)>;
    using FN_QUERY_FEED_DATA = std::function<void(const FeedData&)>;
    bool QueryFeed(long long feedId, FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedByGuid(const std::string &guid, FN_QUERY_FEED fnQueryFeed);
    bool QueryAllFeeds(FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedDataByFeedId(long long feedid, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedDataOrderByTimestamp(long long feedid, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryAllFeedDataOrderByTimestamp(FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedDataByFeedDataId(long long feeddataid, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool DeleteAllFeeds();
    bool DeleteAllFeedData();
    bool UpdateFeedDataReadColumn(long long feeddataid, long long read);
};
