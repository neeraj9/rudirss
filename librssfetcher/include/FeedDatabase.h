#pragma once

#include "SQLite3Handle.h"
#include "SQLite3Stmt.h"

#include <string>
#include <time.h>
#include <functional>
#include <atlcore.h>

class FeedDatabase
{
public:
    FeedDatabase();
    virtual ~FeedDatabase();

    bool Open(const std::wstring &dbPath);
    void Initialize();
    void Close();

    struct Feed
    {
        static const unsigned DEFAULT_FEED_INITIAL_UPDATE_DUETIME = 2 * 60 * 60 * 1000; // milliseconds
        static const unsigned DEFAULT_FEED_UPDATE_DUETIME = 0; // milliseconds
        static const unsigned DEFAULT_FEED_UPDATE_INTERVAL = 12 * 60 * 60 * 1000; // milliseconds

        long long feedid;
        std::string guid;
        std::string url;
        std::string title;
        unsigned duetime;
        unsigned updateinterval;
        Feed() : feedid{ 0 }, duetime{ DEFAULT_FEED_INITIAL_UPDATE_DUETIME }, updateinterval{ DEFAULT_FEED_UPDATE_INTERVAL } {}
        Feed(const Feed& rhs) : feedid{ rhs.feedid }, guid{ rhs.guid }, url{ rhs.url }, title{ rhs.title },
            duetime{ rhs.duetime }, updateinterval{ rhs.updateinterval } {}
        Feed(Feed&& rhs) noexcept : feedid{ rhs.feedid }, guid{ std::move(rhs.guid) }, url{ std::move(rhs.url) }, title{ rhs.title },
            duetime{ rhs.duetime }, updateinterval{ rhs.updateinterval } {}
        Feed& operator=(Feed&& rhs) noexcept
        {
            if (this != &rhs)
            {
                feedid = rhs.feedid;
                guid = std::move(rhs.guid);
                url = std::move(rhs.url);
                title = std::move(rhs.title);
                duetime = rhs.duetime;
                updateinterval = rhs.updateinterval;
            }

            return *this;
        }
        Feed& operator=(const Feed& rhs)
        {
            if (this != &rhs)
            {
                feedid = rhs.feedid;
                guid = rhs.guid;
                url = rhs.url;
                title = rhs.title;
                duetime = rhs.duetime;
                updateinterval = rhs.updateinterval;
            }

            return *this;
        }
    };

    static const long long INVALID_FEED_ID = -1;
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

        FeedData& operator=(const FeedData& rhs)
        {
            if (this != &rhs)
            {
                feeddataid = rhs.feeddataid;
                guid = rhs.guid;
                feedid = rhs.feedid;
                link = rhs.link;
                title = rhs.title;
                datetime = rhs.datetime;
                timestamp = rhs.timestamp;
                createdtime = rhs.createdtime;
                read = rhs.read;
                tag = rhs.tag;
                misc = rhs.misc;
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
    bool QueryFeedByGuid(const std::string& guid, FN_QUERY_FEED fnQueryFeed);
    bool QueryAllFeeds(FN_QUERY_FEED fnQueryFeed);
    bool QueryAllFeedsOrderByTitleASC(FN_QUERY_FEED fnQueryFeed);
    bool QueryAllFeedsOrderByTitleDESC(FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedDataByFeedId(long long feedid, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedDataOrderByTimestamp(long long feedid, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedDataByOffsetOrderByTimestamp(long long offset, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedDataByFeedIdByOffsetOrderByTimestamp(long long feedid, long long offset, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryAllFeedDataOrderByTimestamp(FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedDataByFeedDataId(long long feeddataid, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool DeleteAllFeeds();
    bool DeleteAllFeedData();
    bool DeleteOutdatedFeedData(long long timestamp);
    bool DeleteFeedByFeedId(long long feedid);
    bool DeleteFeedDataByFeedId(long long feedid);
    bool UpdateFeedDataReadColumn(long long feeddataid, long long read);
    bool QueryFeedTableDataExist(long long &exist);
    bool QueryFeedDataTableCount(long long &count);
    bool QueryFeedDataTableCountByFeedId(long long feedid, long long &count);
    bool QueryFeedTableCount(long long &count);
    bool QueryFeedByOffset(long long offset, FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedByOffsetInRange(long long limit, long long offset, FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedByOffsetOrderByTitleASCInRange(long long limit, long long offset, FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedByOffsetOrderByTitleDESCInRange(long long limit, long long offset, FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedDataByOffset(long long offset, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedDataOrderByTimestampInRange(long long limit, long long offset, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedDataByFeedIdOrderByTimestampInRange(long long feedid, long long limit, long long offset, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedExistByGuid(const std::string& guid, long long& exist);
    bool QueryFeedDataCountByTitle(const std::string& title, long long &count);
    bool QueryFeedDataByTitleOrderByTimestampInRange(const std::string &title, long long limit, long long offset, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedDataCountByFeedIdByTitle(long long feedid, const std::string& title, long long &count);
    bool QueryFeedDataByFeedIdByTitleOrderByTimestampInRange(long long feedid, const std::string &title, long long limit, long long offset,
        FN_QUERY_FEED_DATA fnQueryFeedData);
    bool QueryFeedCountByTitle(const std::string& title, long long &count);
    bool QueryFeedByTitleByOffsetInRange(const std::string &title, long long limit, long long offset, FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedByTitleByOffsetOrderByTitleASCInRange(const std::string &title, long long limit, long long offset, FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedByTitleByOffsetOrderByTitleDESCInRange(const std::string &title, long long limit, long long offset, FN_QUERY_FEED fnQueryFeed);

protected:
    SQLite3Handle m_sql;
    SQLite3Stmt m_insertFeedStmt;
    SQLite3Stmt m_insertFeedDataStmt;
    SQLite3Stmt m_queryFeedStmt;
    SQLite3Stmt m_queryFeedByGuidStmt;
    SQLite3Stmt m_queryAllFeedsStmt;
    SQLite3Stmt m_queryFeedDataByFeedIdStmt;
    SQLite3Stmt m_queryFeedDataByFeedDataIdStmt;
    SQLite3Stmt m_queryFeedDataByFeedIdOrderByTimestampStmt;
    SQLite3Stmt m_queryFeedDataByOffsetOrderByTimestampStmt;
    SQLite3Stmt m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt;
    SQLite3Stmt m_queryAllFeedDataOrderByTimestampStmt;
    SQLite3Stmt m_deleteAllFeedStmt;
    SQLite3Stmt m_deleteAllFeedDataStmt;
    SQLite3Stmt m_deleteOutdatedFeedDataStmt;
    SQLite3Stmt m_deleteFeedByFeedIdStmt;
    SQLite3Stmt m_deleteFeedDataByFeedIdStmt;
    SQLite3Stmt m_updateFeedDataReadStmt;
    SQLite3Stmt m_queryFeedTableDataExistStmt;
    SQLite3Stmt m_queryFeedDataTableCountStmt;
    SQLite3Stmt m_queryFeedDataTableCountByFeedIdStmt;
    SQLite3Stmt m_queryFeedDataByOffsetStmt;
    SQLite3Stmt m_queryFeedTableCountStmt;
    SQLite3Stmt m_queryFeedByOffsetStmt;
    SQLite3Stmt m_queryFeedByOffsetInRangeStmt;
    SQLite3Stmt m_queryFeedDataOrderByTimestampInRangeStmt;
    SQLite3Stmt m_queryFeedDataByFeedIdOrderByTimestampInRangeStmt;
    SQLite3Stmt m_queryFeedExistByGuidStmt;
    SQLite3Stmt m_queryFeedDataCountByTitle;
    SQLite3Stmt m_queryFeedDataByTitleOrderByTimestampInRangeStmt;
    SQLite3Stmt m_queryFeedDataCountByFeedIdByTitle;
    SQLite3Stmt m_queryFeedDataByFeedIdByTitleOrderByTimestampInRangeStmt;
    SQLite3Stmt m_queryAllFeedsOrderByTitleASCStmt;
    SQLite3Stmt m_queryAllFeedsOrderByTitleDESCStmt;
    SQLite3Stmt m_queryFeedOrderByTitleASCInRangeStmt;
    SQLite3Stmt m_queryFeedOrderByTitleDESCInRangeStmt;
    SQLite3Stmt m_queryFeedCountByTitleStmt;
    SQLite3Stmt m_queryFeedByTitleByOffsetInRangeStmt;
    SQLite3Stmt m_queryFeedByTitleByOffsetOrderByTitleASCInRangeStmt;
    SQLite3Stmt m_queryFeedByTitleByOffsetOrderByTitleDESCInRangeStmt;

    ATL::CComCriticalSection m_dbLock;

    void QueryFeedData(FeedData& feedData, sqlite3_stmt *stmt);
    void QueryFeed(Feed& feed, sqlite3_stmt *stmt);
};
