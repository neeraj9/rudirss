#pragma once

#include "SQLite3Handle.h"
#include "SQLite3StmtHandle.h"

#include <string>
#include <time.h>
#include <functional>

class FeedDatabase
{
protected:
    SQLite3Handle m_sql;
    SQLite3StmtHandle m_insertFeedStmt;
    SQLite3StmtHandle m_insertFeedDataStmt;
    SQLite3StmtHandle m_queryFeedStmt;
    SQLite3StmtHandle m_queryFeedDataStmt;
    SQLite3StmtHandle m_deleteAllFeedStmt;
    SQLite3StmtHandle m_deleteAllFeedDataStmt;

public:
    FeedDatabase();
    virtual ~FeedDatabase();

    bool Open(const std::wstring &dbPath);
    void Initialize();
    void Close();

    struct Feed
    {
        std::string guid;
        std::string url;
        Feed() {}
        Feed(const Feed& rhs) : guid{ rhs.guid }, url{ rhs.url } {}
        Feed(Feed&& rhs) noexcept: guid{ std::move(rhs.guid) }, url{ std::move(rhs.url) } {}
    };

    struct FeedData
    {
        std::string guid;
        std::string feedguid;
        std::string link;
        std::string title;
        std::string datetime;
        time_t timestamp;
        time_t createdtime;
        std::string tag;
        std::string misc;
        FeedData() : timestamp{ 0 }, createdtime{ 0 } {}
        FeedData(const FeedData& rhs) : guid{ rhs.guid }, feedguid{ rhs.feedguid }, link{ rhs.link }, title{ rhs.title },
            datetime{ rhs.datetime }, timestamp{ rhs.timestamp }, createdtime{ rhs.createdtime }, tag{ rhs.tag },
            misc{ rhs.misc } {}
        FeedData(FeedData&& rhs) noexcept: guid{ std::move(rhs.guid) }, feedguid{ std::move(rhs.feedguid) }, link{ std::move(rhs.link) },
            title{ std::move(rhs.title) }, datetime{ std::move(rhs.datetime) }, timestamp{ rhs.timestamp }, createdtime{ rhs.createdtime },
            tag{ std::move(rhs.tag) }, misc{ std::move(rhs.misc) } {}
    };

    bool InsertFeed(const Feed &feed);
    bool InsertFeedData(const FeedData &feedData);

    using FN_QUERY_FEED = std::function<void(const Feed&)>;
    using FN_QUERY_FEED_DATA = std::function<void(const FeedData&)>;
    bool QueryAllFeeds(FN_QUERY_FEED fnQueryFeed);
    bool QueryFeedData(const std::string &guid, FN_QUERY_FEED_DATA fnQueryFeedData);
    bool DeleteAllFeeds();
    bool DeleteAllFeedData();
};
