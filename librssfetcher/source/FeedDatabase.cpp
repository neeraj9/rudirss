#include "FeedDatabase.h"
#include <stdexcept>
#include <atlbase.h>

FeedDatabase::FeedDatabase()
{
    m_dbLock.Init();
}

FeedDatabase::~FeedDatabase()
{

}

bool FeedDatabase::Open(const std::wstring& dbPath)
{
    return SQLITE_OK == sqlite3_open16(dbPath.c_str(), &m_sql.m_handle);
}

void FeedDatabase::Initialize()
{
    int ret = sqlite3_exec(m_sql.m_handle, "CREATE TABLE IF NOT EXISTS Feed(feedid INTEGER PRIMARY KEY AUTOINCREMENT, guid TEXT NOT NULL, url TEXT NOT NULL,"\
        "title TEXT NOT NULL)", nullptr, nullptr, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot create Feed table.");

    ret = sqlite3_exec(m_sql.m_handle, "CREATE TABLE IF NOT EXISTS FeedData(feeddataid INTEGER PRIMARY KEY AUTOINCREMENT, guid TEXT NOT NULL, feedguid TEXT NOT NULL,"\
        "link TEXT NOT NULL, title TEXT NOT NULL, datetime TEXT NOT NULL, timestamp INTEGER, createdtime INTEGER, tag TEXT, misc TEXT)",
        nullptr, nullptr, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot create FeedData table.");

    std::string stmt = "INSERT INTO Feed(guid, url, title) SELECT ?, ?, ? WHERE NOT EXISTS(SELECT * FROM Feed WHERE guid = ?)";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_insertFeedStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare insertion statement for feed.");

    stmt = "INSERT INTO FeedData(guid, feedguid, link, title, datetime, timestamp, createdtime, tag, misc) SELECT ?, ?, ?, ?, ?, ?, ?, ?, ?"\
        "WHERE NOT EXISTS(SELECT * FROM FeedData WHERE guid = ?)";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_insertFeedDataStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare insertion statement for feed data.");

    stmt = "SELECT * FROM Feed WHERE feedid = ?";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement for feed.");

    stmt = "SELECT * FROM Feed WHERE guid = ?";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedByGuidStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement for feed by guid.");

    stmt = "SELECT * FROM Feed";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryAllFeedsStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement for all feeds.");

    stmt = "SELECT * FROM FeedData WHERE feedguid = ?";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedDataByGuidStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement for feed data by guid.");

    stmt = "SELECT * FROM FeedData WHERE feedguid = ? ORDER BY timestamp DESC";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedDataByGuidOrderByTimestampStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement for feed data by guid order by timestamp.");

    stmt = "SELECT * FROM FeedData WHERE feeddataid = ?";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedDataByFeedDataIdStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement for feed data by feeddataid.");

    stmt = "DELETE FROM Feed";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_deleteAllFeedStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare deletion statement for feed.");

    stmt = "DELETE FROM FeedData";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_deleteAllFeedDataStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare deletion statement for feed data.");
}

void FeedDatabase::Close()
{
    m_insertFeedStmt.Close();
    m_insertFeedDataStmt.Close();
    m_queryFeedStmt.Close();
    m_queryFeedByGuidStmt.Close();
    m_queryAllFeedsStmt.Close();
    m_queryFeedDataByGuidStmt.Close();
    m_queryFeedDataByFeedDataIdStmt.Close();
    m_deleteAllFeedStmt.Close();
    m_deleteAllFeedDataStmt.Close();
    m_sql.Close();
}

bool FeedDatabase::InsertFeed(const Feed& feed)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_insertFeedStmt.m_handle)
        return false;

    int col = 1;
    sqlite3_bind_text(m_insertFeedStmt.m_handle, col++, feed.guid.c_str(), feed.guid.length(), nullptr);
    sqlite3_bind_text(m_insertFeedStmt.m_handle, col++, feed.url.c_str(), feed.url.length(), nullptr);
    sqlite3_bind_text(m_insertFeedStmt.m_handle, col++, feed.title.c_str(), feed.title.length(), nullptr);
    sqlite3_bind_text(m_insertFeedStmt.m_handle, col++, feed.guid.c_str(), feed.guid.length(), nullptr);
    int ret = sqlite3_step(m_insertFeedStmt.m_handle);
    sqlite3_reset(m_insertFeedStmt.m_handle);

    return SQLITE_DONE == ret;
}

bool FeedDatabase::InsertFeedData(const FeedData& feedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_insertFeedDataStmt.m_handle)
        return false;

    int col = 1;
    sqlite3_bind_text(m_insertFeedDataStmt.m_handle, col++, feedData.guid.c_str(), feedData.guid.length(), nullptr);
    sqlite3_bind_text(m_insertFeedDataStmt.m_handle, col++, feedData.feedguid.c_str(), feedData.feedguid.length(), nullptr);
    sqlite3_bind_text(m_insertFeedDataStmt.m_handle, col++, feedData.link.c_str(), feedData.link.length(), nullptr);
    sqlite3_bind_text(m_insertFeedDataStmt.m_handle, col++, feedData.title.c_str(), feedData.title.length(), nullptr);
    sqlite3_bind_text(m_insertFeedDataStmt.m_handle, col++, feedData.datetime.c_str(), feedData.datetime.length(), nullptr);
    sqlite3_bind_int64(m_insertFeedDataStmt.m_handle, col++, feedData.timestamp);
    sqlite3_bind_int64(m_insertFeedDataStmt.m_handle, col++, feedData.createdtime);
    sqlite3_bind_text(m_insertFeedDataStmt.m_handle, col++, feedData.tag.c_str(), feedData.tag.length(), nullptr);
    sqlite3_bind_text(m_insertFeedDataStmt.m_handle, col++, feedData.misc.c_str(), feedData.misc.length(), nullptr);
    sqlite3_bind_text(m_insertFeedDataStmt.m_handle, col++, feedData.guid.c_str(), feedData.guid.length(), nullptr);
    int ret = sqlite3_step(m_insertFeedDataStmt.m_handle);
    sqlite3_reset(m_insertFeedDataStmt.m_handle);

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeed(long long feedId, FN_QUERY_FEED fnQueryFeed)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedStmt.m_handle
        || !fnQueryFeed)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = sqlite3_bind_int64(m_queryFeedStmt.m_handle, 1, feedId)))
    {
        Feed feed;
        while (SQLITE_ROW == (ret = sqlite3_step(m_queryFeedStmt.m_handle)))
        {
            int col = 0;
            feed.feedid = sqlite3_column_int64(m_queryFeedStmt.m_handle, col++);
            feed.guid = (const char*)sqlite3_column_text(m_queryFeedStmt.m_handle, col++);
            feed.url = (const char*)sqlite3_column_text(m_queryFeedStmt.m_handle, col++);
            feed.title = (const char*)sqlite3_column_text(m_queryFeedStmt.m_handle, col++);
            fnQueryFeed(feed);
        }
        sqlite3_reset(m_queryFeedStmt.m_handle);
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeed(const std::string& guid, FN_QUERY_FEED fnQueryFeed)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedByGuidStmt.m_handle
        || !fnQueryFeed)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = sqlite3_bind_text(m_queryFeedByGuidStmt.m_handle, 1, guid.c_str(), guid.length(), nullptr)))
    {
        Feed feed;
        while (SQLITE_ROW == (ret = sqlite3_step(m_queryFeedByGuidStmt.m_handle)))
        {
            int col = 0;
            feed.feedid = sqlite3_column_int64(m_queryFeedByGuidStmt.m_handle, col++);
            feed.guid = (const char*)sqlite3_column_text(m_queryFeedByGuidStmt.m_handle, col++);
            feed.url = (const char*)sqlite3_column_text(m_queryFeedByGuidStmt.m_handle, col++);
            feed.title = (const char*)sqlite3_column_text(m_queryFeedByGuidStmt.m_handle, col++);
            fnQueryFeed(feed);
        }
        sqlite3_reset(m_queryFeedByGuidStmt.m_handle);
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryAllFeeds(FN_QUERY_FEED fnQueryFeed)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryAllFeedsStmt.m_handle
        || !fnQueryFeed)
        return false;

    Feed feed;
    int ret = SQLITE_OK;
    while (SQLITE_ROW == (ret = sqlite3_step(m_queryAllFeedsStmt.m_handle)))
    {
        int col = 0;
        feed.feedid = sqlite3_column_int64(m_queryAllFeedsStmt.m_handle, col++);
        feed.guid = (const char*)sqlite3_column_text(m_queryAllFeedsStmt.m_handle, col++);
        feed.url = (const char*)sqlite3_column_text(m_queryAllFeedsStmt.m_handle, col++);
        feed.title = (const char*)sqlite3_column_text(m_queryAllFeedsStmt.m_handle, col++);
        fnQueryFeed(feed);
    }
    sqlite3_reset(m_queryAllFeedsStmt.m_handle);

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedData(const std::string& guid, FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataByGuidStmt.m_handle
        || !fnQueryFeedData)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = sqlite3_bind_text(m_queryFeedDataByGuidStmt.m_handle, 1, guid.c_str(), guid.length(), nullptr)))
    {
        FeedData feedData;
        while (SQLITE_ROW == (ret = sqlite3_step(m_queryFeedDataByGuidStmt.m_handle)))
        {
            int col = 0;
            feedData.feeddataid = sqlite3_column_int64(m_queryFeedDataByGuidStmt.m_handle, col++);
            feedData.guid = (const char*)sqlite3_column_text(m_queryFeedDataByGuidStmt.m_handle, col++);;
            feedData.feedguid = (const char*)sqlite3_column_text(m_queryFeedDataByGuidStmt.m_handle, col++);
            feedData.link = (const char*)sqlite3_column_text(m_queryFeedDataByGuidStmt.m_handle, col++);
            feedData.title = (const char*)sqlite3_column_text(m_queryFeedDataByGuidStmt.m_handle, col++);
            feedData.datetime = (const char*)sqlite3_column_text(m_queryFeedDataByGuidStmt.m_handle, col++);
            feedData.timestamp = sqlite3_column_int64(m_queryFeedDataByGuidStmt.m_handle, col++);
            feedData.createdtime = sqlite3_column_int64(m_queryFeedDataByGuidStmt.m_handle, col++);
            feedData.tag = (const char*)sqlite3_column_text(m_queryFeedDataByGuidStmt.m_handle, col++);
            feedData.misc = (const char*)sqlite3_column_text(m_queryFeedDataByGuidStmt.m_handle, col++);
            fnQueryFeedData(feedData);
        }
        sqlite3_reset(m_queryFeedDataByGuidStmt.m_handle);
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataOrderByTimestamp(const std::string& guid, FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataByGuidOrderByTimestampStmt.m_handle
        || !fnQueryFeedData)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = sqlite3_bind_text(m_queryFeedDataByGuidOrderByTimestampStmt.m_handle, 1, guid.c_str(), guid.length(), nullptr)))
    {
        FeedData feedData;
        while (SQLITE_ROW == (ret = sqlite3_step(m_queryFeedDataByGuidOrderByTimestampStmt.m_handle)))
        {
            int col = 0;
            feedData.feeddataid = sqlite3_column_int64(m_queryFeedDataByGuidOrderByTimestampStmt.m_handle, col++);
            feedData.guid = (const char*)sqlite3_column_text(m_queryFeedDataByGuidOrderByTimestampStmt.m_handle, col++);;
            feedData.feedguid = (const char*)sqlite3_column_text(m_queryFeedDataByGuidOrderByTimestampStmt.m_handle, col++);
            feedData.link = (const char*)sqlite3_column_text(m_queryFeedDataByGuidOrderByTimestampStmt.m_handle, col++);
            feedData.title = (const char*)sqlite3_column_text(m_queryFeedDataByGuidOrderByTimestampStmt.m_handle, col++);
            feedData.datetime = (const char*)sqlite3_column_text(m_queryFeedDataByGuidOrderByTimestampStmt.m_handle, col++);
            feedData.timestamp = sqlite3_column_int64(m_queryFeedDataByGuidOrderByTimestampStmt.m_handle, col++);
            feedData.createdtime = sqlite3_column_int64(m_queryFeedDataByGuidOrderByTimestampStmt.m_handle, col++);
            feedData.tag = (const char*)sqlite3_column_text(m_queryFeedDataByGuidOrderByTimestampStmt.m_handle, col++);
            feedData.misc = (const char*)sqlite3_column_text(m_queryFeedDataByGuidOrderByTimestampStmt.m_handle, col++);
            fnQueryFeedData(feedData);
        }
        sqlite3_reset(m_queryFeedDataByGuidOrderByTimestampStmt.m_handle);
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedData(long long feeddataid, FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataByFeedDataIdStmt.m_handle
        || !fnQueryFeedData)
        return false;

    int ret = SQLITE_OK;;
    if (SQLITE_OK == (ret = sqlite3_bind_int64(m_queryFeedDataByFeedDataIdStmt.m_handle, 1, feeddataid)))
    {
        FeedData feedData;
        while (SQLITE_ROW == (ret = sqlite3_step(m_queryFeedDataByFeedDataIdStmt.m_handle)))
        {
            int col = 0;
            feedData.feeddataid = sqlite3_column_int64(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
            feedData.guid = (const char*)sqlite3_column_text(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);;
            feedData.feedguid = (const char*)sqlite3_column_text(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
            feedData.link = (const char*)sqlite3_column_text(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
            feedData.title = (const char*)sqlite3_column_text(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
            feedData.datetime = (const char*)sqlite3_column_text(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
            feedData.timestamp = sqlite3_column_int64(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
            feedData.createdtime = sqlite3_column_int64(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
            feedData.tag = (const char*)sqlite3_column_text(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
            feedData.misc = (const char*)sqlite3_column_text(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
            fnQueryFeedData(feedData);
        }
        sqlite3_reset(m_queryFeedDataByFeedDataIdStmt.m_handle);
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::DeleteAllFeeds()
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_deleteAllFeedStmt.m_handle)
        return false;

    int ret = sqlite3_step(m_deleteAllFeedStmt.m_handle);
    sqlite3_reset(m_deleteAllFeedStmt.m_handle);

    return SQLITE_DONE == ret;
}

bool FeedDatabase::DeleteAllFeedData()
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_deleteAllFeedDataStmt.m_handle)
        return false;

    int ret = sqlite3_step(m_deleteAllFeedDataStmt.m_handle);
    sqlite3_reset(m_deleteAllFeedDataStmt.m_handle);

    return SQLITE_DONE == ret;
}
