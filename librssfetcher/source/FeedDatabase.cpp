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
        "title TEXT NOT NULL, duetime INTEGER NOT NULL, updateinterval INTEGER NOT NULL)", nullptr, nullptr, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot create Feed table.");

    ret = sqlite3_exec(m_sql.m_handle, "CREATE TABLE IF NOT EXISTS FeedData(feeddataid INTEGER PRIMARY KEY AUTOINCREMENT, guid TEXT NOT NULL, feedid INTEGER NOT NULL,"\
        "link TEXT NOT NULL, title TEXT NOT NULL, datetime TEXT NOT NULL, timestamp INTEGER, createdtime INTEGER, read INTEGER, tag TEXT, misc TEXT)",
        nullptr, nullptr, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot create FeedData table.");

    std::string stmt = "INSERT INTO Feed(guid, url, title, duetime, updateinterval) SELECT ?, ?, ?, ?, ? WHERE NOT EXISTS(SELECT * FROM Feed WHERE guid = ?)";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_insertFeedStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare insertion statement for feed.");

    stmt = "INSERT INTO FeedData(guid, feedid, link, title, datetime, timestamp, createdtime, read, tag, misc) SELECT ?, ?, ?, ?, ?, ?, ?, ?, ?, ?"\
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
        throw std::runtime_error("Error: cannot prepare query statement by guid for feed.");

    stmt = "SELECT * FROM Feed";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryAllFeedsStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement for all feeds.");

    stmt = "SELECT * FROM FeedData WHERE feedid = ?";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedDataByFeedIdStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement by feedid for feed data.");

    stmt = "SELECT * FROM FeedData WHERE feedid = ? ORDER BY timestamp DESC";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement for feed data order by timestamp.");

    stmt = "SELECT * FROM FeedData ORDER BY timestamp DESC LIMIT 1 OFFSET ?";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement by offset for feed data order by timestamp.");

    stmt = "SELECT * FROM FeedData WHERE feedid = ? ORDER BY timestamp DESC LIMIT 1 OFFSET ?";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement by feedid and offset for feed data order by timestamp.");

    stmt = "SELECT * FROM FeedData ORDER BY timestamp DESC";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryAllFeedDataOrderByTimestampStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement for all feed data.");

    stmt = "SELECT * FROM FeedData WHERE feeddataid = ?";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedDataByFeedDataIdStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement by feeddataid for feed data.");

    stmt = "DELETE FROM Feed";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_deleteAllFeedStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare deletion statement for feed.");

    stmt = "DELETE FROM FeedData";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_deleteAllFeedDataStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare deletion statement for all feed data.");

    stmt = "DELETE FROM FeedData WHERE timestamp < ?";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_deleteOutdatedFeedDataStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare deletion statement for outdated feed data.");

    stmt = "DELETE FROM Feed WHERE feedid = ?";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_deleteFeedByFeedIdStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare deletion statement by feedid for feed.");

    stmt = "DELETE FROM FeedData WHERE feedid = ?";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_deleteFeedDataByFeedIdStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare deletion statement by feedid for feed data.");

    stmt = "UPDATE FeedData SET read = ? WHERE feeddataid = ?";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_updateFeedDataReadStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare update statement for read column of feed data.");

    stmt = "SELECT EXISTS (SELECT 1 FROM Feed LIMIT 1)";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedTableDataExistStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement of checking whether feed table is empty.");

    stmt = "SELECT Count(*) FROM FeedData";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedDataTableCountStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement of feed data count.");

    stmt = "SELECT Count(*) FROM FeedData WHERE feedid = ?";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedDataTableCountByFeedIdStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement of feed data count by feedid.");

    stmt = "SELECT * FROM FeedData LIMIT 1 OFFSET ?";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedDataByOffsetStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement by offset for feed data.");

    stmt = "SELECT Count(*) FROM Feed";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedTableCountStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement of feed count.");

    stmt = "SELECT * FROM Feed LIMIT 1 OFFSET ?";
    ret = sqlite3_prepare_v2(m_sql.m_handle, stmt.c_str(), stmt.length(), &m_queryFeedByOffsetStmt.m_handle, nullptr);
    if (0 != ret)
        throw std::runtime_error("Error: cannot prepare query statement by offset for feed.");
}

void FeedDatabase::Close()
{
    m_insertFeedStmt.Close();
    m_insertFeedDataStmt.Close();
    m_queryFeedStmt.Close();
    m_queryFeedByGuidStmt.Close();
    m_queryAllFeedsStmt.Close();
    m_queryFeedDataByFeedIdStmt.Close();
    m_queryFeedDataByFeedDataIdStmt.Close();
    m_queryFeedDataByFeedIdOrderByTimestampStmt.Close();
    m_queryFeedDataByOffsetOrderByTimestampStmt.Close();
    m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.Close();
    m_queryAllFeedDataOrderByTimestampStmt.Close();
    m_deleteAllFeedStmt.Close();
    m_deleteAllFeedDataStmt.Close();
    m_deleteOutdatedFeedDataStmt.Close();
    m_deleteFeedByFeedIdStmt.Close();
    m_deleteFeedDataByFeedIdStmt.Close();
    m_updateFeedDataReadStmt.Close();
    m_queryFeedTableDataExistStmt.Close();
    m_queryFeedDataTableCountStmt.Close();
    m_queryFeedDataTableCountByFeedIdStmt.Close();
    m_queryFeedDataByOffsetStmt.Close();
    m_queryFeedTableCountStmt.Close();
    m_queryFeedByOffsetStmt.Close();

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
    sqlite3_bind_int64(m_insertFeedStmt.m_handle, col++, static_cast<long long>(feed.duetime));
    sqlite3_bind_int64(m_insertFeedStmt.m_handle, col++, static_cast<long long>(feed.updateinterval));
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
    sqlite3_bind_int64(m_insertFeedDataStmt.m_handle, col++, feedData.feedid);
    sqlite3_bind_text(m_insertFeedDataStmt.m_handle, col++, feedData.link.c_str(), feedData.link.length(), nullptr);
    sqlite3_bind_text(m_insertFeedDataStmt.m_handle, col++, feedData.title.c_str(), feedData.title.length(), nullptr);
    sqlite3_bind_text(m_insertFeedDataStmt.m_handle, col++, feedData.datetime.c_str(), feedData.datetime.length(), nullptr);
    sqlite3_bind_int64(m_insertFeedDataStmt.m_handle, col++, feedData.timestamp);
    sqlite3_bind_int64(m_insertFeedDataStmt.m_handle, col++, feedData.createdtime);
    sqlite3_bind_int64(m_insertFeedDataStmt.m_handle, col++, feedData.read);
    sqlite3_bind_text(m_insertFeedDataStmt.m_handle, col++, feedData.tag.c_str(), feedData.tag.length(), nullptr);
    sqlite3_bind_text(m_insertFeedDataStmt.m_handle, col++, feedData.misc.c_str(), feedData.misc.length(), nullptr);
    sqlite3_bind_text(m_insertFeedDataStmt.m_handle, col++, feedData.guid.c_str(), feedData.guid.length(), nullptr);
    int ret = sqlite3_step(m_insertFeedDataStmt.m_handle);
    sqlite3_reset(m_insertFeedDataStmt.m_handle);

    return SQLITE_DONE == ret;
}

sqlite3_int64 FeedDatabase::GetLastInsertRowid()
{
    ATL::CComCritSecLock lock(m_dbLock);
    return sqlite3_last_insert_rowid(m_sql.m_handle);
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
            feed.duetime = static_cast<unsigned>(sqlite3_column_int64(m_queryFeedStmt.m_handle, col++));
            feed.updateinterval = static_cast<unsigned>(sqlite3_column_int64(m_queryFeedStmt.m_handle, col++));
            fnQueryFeed(feed);
        }
        sqlite3_reset(m_queryFeedStmt.m_handle);
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedByGuid(const std::string& guid, FN_QUERY_FEED fnQueryFeed)
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
            feed.duetime = static_cast<unsigned>(sqlite3_column_int64(m_queryFeedByGuidStmt.m_handle, col++));
            feed.updateinterval = static_cast<unsigned>(sqlite3_column_int64(m_queryFeedByGuidStmt.m_handle, col++));
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
        feed.duetime = static_cast<unsigned>(sqlite3_column_int64(m_queryAllFeedsStmt.m_handle, col++));
        feed.updateinterval = static_cast<unsigned>(sqlite3_column_int64(m_queryAllFeedsStmt.m_handle, col++));
        fnQueryFeed(feed);
    }
    sqlite3_reset(m_queryAllFeedsStmt.m_handle);

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataByFeedId(long long feedid, FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataByFeedIdStmt.m_handle
        || !fnQueryFeedData)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = sqlite3_bind_int64(m_queryFeedDataByFeedIdStmt.m_handle, 1, feedid)))
    {
        FeedData feedData;
        while (SQLITE_ROW == (ret = sqlite3_step(m_queryFeedDataByFeedIdStmt.m_handle)))
        {
            int col = 0;
            feedData.feeddataid = sqlite3_column_int64(m_queryFeedDataByFeedIdStmt.m_handle, col++);
            feedData.guid = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdStmt.m_handle, col++);;
            feedData.feedid = sqlite3_column_int64(m_queryFeedDataByFeedIdStmt.m_handle, col++);
            feedData.link = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdStmt.m_handle, col++);
            feedData.title = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdStmt.m_handle, col++);
            feedData.datetime = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdStmt.m_handle, col++);
            feedData.timestamp = sqlite3_column_int64(m_queryFeedDataByFeedIdStmt.m_handle, col++);
            feedData.createdtime = sqlite3_column_int64(m_queryFeedDataByFeedIdStmt.m_handle, col++);
            feedData.read = sqlite3_column_int64(m_queryFeedDataByFeedIdStmt.m_handle, col++);
            feedData.tag = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdStmt.m_handle, col++);
            feedData.misc = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdStmt.m_handle, col++);
            fnQueryFeedData(feedData);
        }
        sqlite3_reset(m_queryFeedDataByFeedIdStmt.m_handle);
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataOrderByTimestamp(long long feedid, FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle
        || !fnQueryFeedData)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = sqlite3_bind_int64(m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle, 1, feedid)))
    {
        FeedData feedData;
        while (SQLITE_ROW == (ret = sqlite3_step(m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle)))
        {
            int col = 0;
            feedData.feeddataid = sqlite3_column_int64(m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle, col++);
            feedData.guid = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle, col++);;
            feedData.feedid = sqlite3_column_int64(m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle, col++);
            feedData.link = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle, col++);
            feedData.title = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle, col++);
            feedData.datetime = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle, col++);
            feedData.timestamp = sqlite3_column_int64(m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle, col++);
            feedData.createdtime = sqlite3_column_int64(m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle, col++);
            feedData.read = sqlite3_column_int64(m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle, col++);
            feedData.tag = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle, col++);
            feedData.misc = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle, col++);
            fnQueryFeedData(feedData);
        }
        sqlite3_reset(m_queryFeedDataByFeedIdOrderByTimestampStmt.m_handle);
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataByOffsetOrderByTimestamp(long long offset, FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle
        || !fnQueryFeedData)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = sqlite3_bind_int64(m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle, 1, offset)))
    {
        FeedData feedData;
        while (SQLITE_ROW == (ret = sqlite3_step(m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle)))
        {
            int col = 0;
            feedData.feeddataid = sqlite3_column_int64(m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.guid = (const char*)sqlite3_column_text(m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle, col++);;
            feedData.feedid = sqlite3_column_int64(m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.link = (const char*)sqlite3_column_text(m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.title = (const char*)sqlite3_column_text(m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.datetime = (const char*)sqlite3_column_text(m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.timestamp = sqlite3_column_int64(m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.createdtime = sqlite3_column_int64(m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.read = sqlite3_column_int64(m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.tag = (const char*)sqlite3_column_text(m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.misc = (const char*)sqlite3_column_text(m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle, col++);
            fnQueryFeedData(feedData);
        }
        sqlite3_reset(m_queryFeedDataByOffsetOrderByTimestampStmt.m_handle);
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataByFeedIdByOffsetOrderByTimestamp(long long feedid, long long offset, FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle
        || !fnQueryFeedData)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = sqlite3_bind_int64(m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle, 1, feedid))
        && SQLITE_OK == (ret = sqlite3_bind_int64(m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle, 2, offset)))
    {
        FeedData feedData;
        while (SQLITE_ROW == (ret = sqlite3_step(m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle)))
        {
            int col = 0;
            feedData.feeddataid = sqlite3_column_int64(m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.guid = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle, col++);;
            feedData.feedid = sqlite3_column_int64(m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.link = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.title = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.datetime = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.timestamp = sqlite3_column_int64(m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.createdtime = sqlite3_column_int64(m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.read = sqlite3_column_int64(m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.tag = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle, col++);
            feedData.misc = (const char*)sqlite3_column_text(m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle, col++);
            fnQueryFeedData(feedData);
        }
        sqlite3_reset(m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.m_handle);
    }

    return SQLITE_DONE == ret;

}

bool FeedDatabase::QueryAllFeedDataOrderByTimestamp(FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryAllFeedDataOrderByTimestampStmt.m_handle
        || !fnQueryFeedData)
        return false;

    int ret = SQLITE_OK;
    FeedData feedData;
    while (SQLITE_ROW == (ret = sqlite3_step(m_queryAllFeedDataOrderByTimestampStmt.m_handle)))
    {
        int col = 0;
        feedData.feeddataid = sqlite3_column_int64(m_queryAllFeedDataOrderByTimestampStmt.m_handle, col++);
        feedData.guid = (const char*)sqlite3_column_text(m_queryAllFeedDataOrderByTimestampStmt.m_handle, col++);;
        feedData.feedid = sqlite3_column_int64(m_queryAllFeedDataOrderByTimestampStmt.m_handle, col++);
        feedData.link = (const char*)sqlite3_column_text(m_queryAllFeedDataOrderByTimestampStmt.m_handle, col++);
        feedData.title = (const char*)sqlite3_column_text(m_queryAllFeedDataOrderByTimestampStmt.m_handle, col++);
        feedData.datetime = (const char*)sqlite3_column_text(m_queryAllFeedDataOrderByTimestampStmt.m_handle, col++);
        feedData.timestamp = sqlite3_column_int64(m_queryAllFeedDataOrderByTimestampStmt.m_handle, col++);
        feedData.createdtime = sqlite3_column_int64(m_queryAllFeedDataOrderByTimestampStmt.m_handle, col++);
        feedData.read = sqlite3_column_int64(m_queryAllFeedDataOrderByTimestampStmt.m_handle, col++);
        feedData.tag = (const char*)sqlite3_column_text(m_queryAllFeedDataOrderByTimestampStmt.m_handle, col++);
        feedData.misc = (const char*)sqlite3_column_text(m_queryAllFeedDataOrderByTimestampStmt.m_handle, col++);
        fnQueryFeedData(feedData);
    }
    sqlite3_reset(m_queryAllFeedDataOrderByTimestampStmt.m_handle);

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataByFeedDataId(long long feeddataid, FN_QUERY_FEED_DATA fnQueryFeedData)
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
            feedData.feedid = sqlite3_column_int64(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
            feedData.link = (const char*)sqlite3_column_text(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
            feedData.title = (const char*)sqlite3_column_text(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
            feedData.datetime = (const char*)sqlite3_column_text(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
            feedData.timestamp = sqlite3_column_int64(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
            feedData.createdtime = sqlite3_column_int64(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
            feedData.read = sqlite3_column_int64(m_queryFeedDataByFeedDataIdStmt.m_handle, col++);
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

bool FeedDatabase::DeleteOutdatedFeedData(long long timestamp)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_deleteOutdatedFeedDataStmt.m_handle)
        return false;

    int ret = SQLITE_OK;;
    if (SQLITE_OK == (ret = sqlite3_bind_int64(m_deleteOutdatedFeedDataStmt.m_handle, 1, timestamp)))
    {
        ret = sqlite3_step(m_deleteOutdatedFeedDataStmt.m_handle);
    }
    sqlite3_reset(m_deleteOutdatedFeedDataStmt.m_handle);

    return SQLITE_DONE == ret;
}

bool FeedDatabase::DeleteFeedByFeedId(long long feedid)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_deleteFeedByFeedIdStmt.m_handle)
        return false;

    int ret = SQLITE_OK;;
    if (SQLITE_OK == (ret = sqlite3_bind_int64(m_deleteFeedByFeedIdStmt.m_handle, 1, feedid)))
    {
        ret = sqlite3_step(m_deleteFeedByFeedIdStmt.m_handle);
    }
    sqlite3_reset(m_deleteFeedByFeedIdStmt.m_handle);

    return SQLITE_DONE == ret;
}

bool FeedDatabase::DeleteFeedDataByFeedId(long long feedid)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_deleteFeedDataByFeedIdStmt.m_handle)
        return false;

    int ret = SQLITE_OK;;
    if (SQLITE_OK == (ret = sqlite3_bind_int64(m_deleteFeedDataByFeedIdStmt.m_handle, 1, feedid)))
    {
        ret = sqlite3_step(m_deleteFeedDataByFeedIdStmt.m_handle);
    }
    sqlite3_reset(m_deleteFeedDataByFeedIdStmt.m_handle);

    return SQLITE_DONE == ret;
}

bool FeedDatabase::UpdateFeedDataReadColumn(long long feeddataid, long long read)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_updateFeedDataReadStmt.m_handle)
        return false;

    int ret = SQLITE_OK;;
    do
    {
        if (SQLITE_OK != (ret = sqlite3_bind_int64(m_updateFeedDataReadStmt.m_handle, 1, read)))
            break;

        if (SQLITE_OK != (ret = sqlite3_bind_int64(m_updateFeedDataReadStmt.m_handle, 2, feeddataid)))
            break;

        ret = sqlite3_step(m_updateFeedDataReadStmt.m_handle);
    } while (0);
    sqlite3_reset(m_updateFeedDataReadStmt.m_handle);

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedTableDataExist(long long& exist)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedTableDataExistStmt.m_handle)
        return false;

    int ret = SQLITE_OK;;
    while (SQLITE_ROW == (ret = sqlite3_step(m_queryFeedTableDataExistStmt.m_handle)))
    {
        exist = sqlite3_column_int64(m_queryFeedTableDataExistStmt.m_handle, 0);
    }
    sqlite3_reset(m_queryFeedTableDataExistStmt.m_handle);

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataTableCount(long long& count)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataTableCountStmt.m_handle)
        return false;

    int ret = SQLITE_OK;;
    while (SQLITE_ROW == (ret = sqlite3_step(m_queryFeedDataTableCountStmt.m_handle)))
    {
        count = sqlite3_column_int64(m_queryFeedDataTableCountStmt.m_handle, 0);
    }
    sqlite3_reset(m_queryFeedDataTableCountStmt.m_handle);

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataTableCountByFeedId(long long feedid, long long& count)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataTableCountByFeedIdStmt.m_handle)
        return false;

    int ret = SQLITE_OK;;
    if (SQLITE_OK == (ret = sqlite3_bind_int64(m_queryFeedDataTableCountByFeedIdStmt.m_handle, 1, feedid)))
    {
        while (SQLITE_ROW == (ret = sqlite3_step(m_queryFeedDataTableCountByFeedIdStmt.m_handle)))
        {
            count = sqlite3_column_int64(m_queryFeedDataTableCountByFeedIdStmt.m_handle, 0);
        }
        sqlite3_reset(m_queryFeedDataTableCountByFeedIdStmt.m_handle);
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedTableCount(long long& count)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedTableCountStmt.m_handle)
        return false;

    int ret = SQLITE_OK;;
    while (SQLITE_ROW == (ret = sqlite3_step(m_queryFeedTableCountStmt.m_handle)))
    {
        count = sqlite3_column_int64(m_queryFeedTableCountStmt.m_handle, 0);
    }
    sqlite3_reset(m_queryFeedTableCountStmt.m_handle);

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedByOffset(long long offset, FN_QUERY_FEED fnQueryFeed)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedByOffsetStmt.m_handle
        || !fnQueryFeed)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = sqlite3_bind_int64(m_queryFeedByOffsetStmt.m_handle, 1, offset)))
    {
        Feed feed;
        while (SQLITE_ROW == (ret = sqlite3_step(m_queryFeedByOffsetStmt.m_handle)))
        {
            int col = 0;
            feed.feedid = sqlite3_column_int64(m_queryFeedByOffsetStmt.m_handle, col++);
            feed.guid = (const char*)sqlite3_column_text(m_queryFeedByOffsetStmt.m_handle, col++);
            feed.url = (const char*)sqlite3_column_text(m_queryFeedByOffsetStmt.m_handle, col++);
            feed.title = (const char*)sqlite3_column_text(m_queryFeedByOffsetStmt.m_handle, col++);
            feed.duetime = static_cast<unsigned>(sqlite3_column_int64(m_queryFeedByOffsetStmt.m_handle, col++));
            feed.updateinterval = static_cast<unsigned>(sqlite3_column_int64(m_queryFeedByOffsetStmt.m_handle, col++));
            fnQueryFeed(feed);
        }
        sqlite3_reset(m_queryFeedByOffsetStmt.m_handle);
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataByOffset(long long offset, FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataByOffsetStmt.m_handle
        || !fnQueryFeedData)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = sqlite3_bind_int64(m_queryFeedDataByOffsetStmt.m_handle, 1, offset)))
    {
        FeedData feedData;
        while (SQLITE_ROW == (ret = sqlite3_step(m_queryFeedDataByOffsetStmt.m_handle)))
        {
            int col = 0;
            feedData.feeddataid = sqlite3_column_int64(m_queryFeedDataByOffsetStmt.m_handle, col++);
            feedData.guid = (const char*)sqlite3_column_text(m_queryFeedDataByOffsetStmt.m_handle, col++);;
            feedData.feedid = sqlite3_column_int64(m_queryFeedDataByOffsetStmt.m_handle, col++);
            feedData.link = (const char*)sqlite3_column_text(m_queryFeedDataByOffsetStmt.m_handle, col++);
            feedData.title = (const char*)sqlite3_column_text(m_queryFeedDataByOffsetStmt.m_handle, col++);
            feedData.datetime = (const char*)sqlite3_column_text(m_queryFeedDataByOffsetStmt.m_handle, col++);
            feedData.timestamp = sqlite3_column_int64(m_queryFeedDataByOffsetStmt.m_handle, col++);
            feedData.createdtime = sqlite3_column_int64(m_queryFeedDataByOffsetStmt.m_handle, col++);
            feedData.read = sqlite3_column_int64(m_queryFeedDataByOffsetStmt.m_handle, col++);
            feedData.tag = (const char*)sqlite3_column_text(m_queryFeedDataByOffsetStmt.m_handle, col++);
            feedData.misc = (const char*)sqlite3_column_text(m_queryFeedDataByOffsetStmt.m_handle, col++);
            fnQueryFeedData(feedData);
        }
        sqlite3_reset(m_queryFeedDataByOffsetStmt.m_handle);
    }

    return SQLITE_DONE == ret;
}
