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

void FeedDatabase::QueryFeedData(FeedData& feedData, sqlite3_stmt* stmt)
{
    int col = 0;
    feedData.feeddataid = sqlite3_column_int64(stmt, col++);
    feedData.guid = (const char*)sqlite3_column_text(stmt, col++);;
    feedData.feedid = sqlite3_column_int64(stmt, col++);
    feedData.link = (const char*)sqlite3_column_text(stmt, col++);
    feedData.title = (const char*)sqlite3_column_text(stmt, col++);
    feedData.datetime = (const char*)sqlite3_column_text(stmt, col++);
    feedData.timestamp = sqlite3_column_int64(stmt, col++);
    feedData.createdtime = sqlite3_column_int64(stmt, col++);
    feedData.read = sqlite3_column_int64(stmt, col++);
    feedData.tag = (const char*)sqlite3_column_text(stmt, col++);
    feedData.misc = (const char*)sqlite3_column_text(stmt, col++);
}

void FeedDatabase::QueryFeed(Feed& feed, sqlite3_stmt* stmt)
{
    int col = 0;
    feed.feedid = sqlite3_column_int64(stmt, col++);
    feed.guid = (const char*)sqlite3_column_text(stmt, col++);
    feed.url = (const char*)sqlite3_column_text(stmt, col++);
    feed.title = (const char*)sqlite3_column_text(stmt, col++);
#ifdef UPDATE_FEED_DUE_TIME_AND_INTERVAL_FROM_DB
    feed.duetime = static_cast<unsigned>(sqlite3_column_int64(stmt, col++));
    feed.updateinterval = static_cast<unsigned>(sqlite3_column_int64(stmt, col++));
#else
    // Dont change duetime and update interval from database
    // Use the default value in FeedDatabase.h
#endif
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

    m_insertFeedStmt.Prepare(m_sql.m_handle, "INSERT INTO Feed(guid, url, title, duetime, updateinterval) SELECT ?, ?, ?, ?, ? WHERE NOT EXISTS(SELECT * FROM Feed WHERE guid = ?)");
    m_insertFeedDataStmt.Prepare(m_sql.m_handle, "INSERT INTO FeedData(guid, feedid, link, title, datetime, timestamp, createdtime, read, tag, misc) "\
        "SELECT ? , ? , ? , ? , ? , ? , ? , ? , ? , ? WHERE NOT EXISTS(SELECT * FROM FeedData WHERE guid = ?)");
    m_queryFeedStmt.Prepare(m_sql.m_handle, "SELECT * FROM Feed WHERE feedid = ?");
    m_queryFeedByGuidStmt.Prepare(m_sql.m_handle, "SELECT * FROM Feed WHERE guid = ?");
    m_queryAllFeedsStmt.Prepare(m_sql.m_handle, "SELECT * FROM Feed");
    m_queryAllFeedsOrderByTitleASCStmt.Prepare(m_sql.m_handle, "SELECT * FROM Feed ORDER BY title ASC");
    m_queryAllFeedsOrderByTitleDESCStmt.Prepare(m_sql.m_handle, "SELECT * FROM Feed ORDER BY title DESC");
    m_queryFeedDataByFeedIdStmt.Prepare(m_sql.m_handle, "SELECT * FROM FeedData WHERE feedid = ?");
    m_queryFeedDataByFeedIdOrderByTimestampStmt.Prepare(m_sql.m_handle, "SELECT * FROM FeedData WHERE feedid = ? ORDER BY timestamp DESC");
    m_queryFeedDataByOffsetOrderByTimestampStmt.Prepare(m_sql.m_handle, "SELECT * FROM FeedData ORDER BY timestamp DESC LIMIT 1 OFFSET ?");
    m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.Prepare(m_sql.m_handle, "SELECT * FROM FeedData WHERE feedid = ? ORDER BY timestamp DESC LIMIT 1 OFFSET ?");
    m_queryAllFeedDataOrderByTimestampStmt.Prepare(m_sql.m_handle, "SELECT * FROM FeedData ORDER BY timestamp DESC");
    m_queryFeedDataByFeedDataIdStmt.Prepare(m_sql.m_handle, "SELECT * FROM FeedData WHERE feeddataid = ?");
    m_deleteAllFeedStmt.Prepare(m_sql.m_handle, "DELETE FROM Feed");
    m_deleteAllFeedDataStmt.Prepare(m_sql.m_handle, "DELETE FROM FeedData");
    m_deleteOutdatedFeedDataStmt.Prepare(m_sql.m_handle, "DELETE FROM FeedData WHERE timestamp < ?");
    m_deleteFeedByFeedIdStmt.Prepare(m_sql.m_handle, "DELETE FROM Feed WHERE feedid = ?");
    m_deleteFeedDataByFeedIdStmt.Prepare(m_sql.m_handle, "DELETE FROM FeedData WHERE feedid = ?");
    m_updateFeedDataReadStmt.Prepare(m_sql.m_handle, "UPDATE FeedData SET read = ? WHERE feeddataid = ?");
    m_queryFeedTableDataExistStmt.Prepare(m_sql.m_handle, "SELECT EXISTS (SELECT 1 FROM Feed LIMIT 1)");
    m_queryFeedDataTableCountStmt.Prepare(m_sql.m_handle, "SELECT Count(*) FROM FeedData");
    m_queryFeedDataTableCountByFeedIdStmt.Prepare(m_sql.m_handle, "SELECT Count(*) FROM FeedData WHERE feedid = ?");
    m_queryFeedDataByOffsetStmt.Prepare(m_sql.m_handle, "SELECT * FROM FeedData LIMIT 1 OFFSET ?");
    m_queryFeedTableCountStmt.Prepare(m_sql.m_handle, "SELECT Count(*) FROM Feed");
    m_queryFeedByOffsetStmt.Prepare(m_sql.m_handle, "SELECT * FROM Feed LIMIT 1 OFFSET ?");
    m_queryFeedByOffsetInRangeStmt.Prepare(m_sql.m_handle, "SELECT * FROM Feed LIMIT ? OFFSET ?");
    m_queryFeedOrderByTitleASCInRangeStmt.Prepare(m_sql.m_handle, "SELECT * FROM Feed ORDER BY title ASC LIMIT ? OFFSET ?");
    m_queryFeedOrderByTitleDESCInRangeStmt.Prepare(m_sql.m_handle, "SELECT * FROM Feed ORDER BY title DESC LIMIT ? OFFSET ?");
    m_queryFeedDataOrderByTimestampInRangeStmt.Prepare(m_sql.m_handle, "SELECT * FROM FeedData ORDER BY timestamp DESC LIMIT ? OFFSET ?");
    m_queryFeedDataByFeedIdOrderByTimestampInRangeStmt.Prepare(m_sql.m_handle, "SELECT * FROM FeedData WHERE feedid = ? ORDER BY timestamp DESC LIMIT ? OFFSET ?");
    m_queryFeedExistByGuidStmt.Prepare(m_sql.m_handle, "SELECT EXISTS (SELECT 1 FROM Feed WHERE guid = ?)");
    m_queryFeedDataCountByTitle.Prepare(m_sql.m_handle, "SELECT COUNT(*) FROM FeedData WHERE title LIKE ?");
    m_queryFeedDataByTitleOrderByTimestampInRangeStmt.Prepare(m_sql.m_handle, "SELECT * FROM FeedData WHERE title LIKE ? ORDER BY timestamp DESC LIMIT ? OFFSET ?");
    m_queryFeedDataCountByFeedIdByTitle.Prepare(m_sql.m_handle, "SELECT COUNT(*) FROM FeedData WHERE feedid = ? AND title LIKE ?");
    m_queryFeedDataByFeedIdByTitleOrderByTimestampInRangeStmt.Prepare(m_sql.m_handle, "SELECT * FROM FeedData WHERE feedid = ? AND title LIKE ? ORDER BY timestamp DESC LIMIT ? OFFSET ?");
    m_queryFeedCountByTitleStmt.Prepare(m_sql.m_handle, "SELECT COUNT(*) FROM Feed WHERE title LIKE ?");
    m_queryFeedByTitleByOffsetInRangeStmt.Prepare(m_sql.m_handle, "SELECT * FROM Feed WHERE title LIKE ? LIMIT ? OFFSET ?");
    m_queryFeedByTitleByOffsetOrderByTitleASCInRangeStmt.Prepare(m_sql.m_handle, "SELECT * FROM Feed WHERE title LIKE ?  ORDER BY title ASC LIMIT ? OFFSET ?");
    m_queryFeedByTitleByOffsetOrderByTitleDESCInRangeStmt.Prepare(m_sql.m_handle, "SELECT * FROM Feed WHERE title LIKE ?  ORDER BY title DESC LIMIT ? OFFSET ?");
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
    m_queryFeedByOffsetInRangeStmt.Close();
    m_queryFeedDataOrderByTimestampInRangeStmt.Close();
    m_queryFeedDataByFeedIdOrderByTimestampInRangeStmt.Close();
    m_queryFeedExistByGuidStmt.Close();
    m_queryFeedDataCountByTitle.Close();
    m_queryFeedDataByTitleOrderByTimestampInRangeStmt.Close();
    m_queryFeedDataCountByFeedIdByTitle.Close();
    m_queryFeedDataByFeedIdByTitleOrderByTimestampInRangeStmt.Close();
    m_queryAllFeedsOrderByTitleASCStmt.Close();
    m_queryAllFeedsOrderByTitleDESCStmt.Close();
    m_queryFeedOrderByTitleASCInRangeStmt.Close();
    m_queryFeedOrderByTitleDESCInRangeStmt.Close();
    m_queryFeedCountByTitleStmt.Close();
    m_queryFeedByTitleByOffsetInRangeStmt.Close();
    m_queryFeedByTitleByOffsetOrderByTitleASCInRangeStmt.Close();
    m_queryFeedByTitleByOffsetOrderByTitleDESCInRangeStmt.Close();

    m_sql.Close();
}

bool FeedDatabase::InsertFeed(const Feed& feed)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_insertFeedStmt.m_handle)
        return false;

    int col = 1;
    m_insertFeedStmt.BindText(feed.guid, col++);
    m_insertFeedStmt.BindText(feed.url, col++);
    m_insertFeedStmt.BindText(feed.title, col++);
    m_insertFeedStmt.BindInt64(static_cast<long long>(feed.duetime), col++);
    m_insertFeedStmt.BindInt64(static_cast<long long>(feed.updateinterval), col++);
    m_insertFeedStmt.BindText(feed.guid, col++);
    int ret = m_insertFeedStmt.Step();

    return SQLITE_DONE == ret;
}

bool FeedDatabase::InsertFeedData(const FeedData& feedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_insertFeedDataStmt.m_handle)
        return false;

    int col = 1;
    m_insertFeedDataStmt.BindText(feedData.guid, col++);
    m_insertFeedDataStmt.BindInt64(feedData.feedid, col++);
    m_insertFeedDataStmt.BindText(feedData.link, col++);
    m_insertFeedDataStmt.BindText(feedData.title, col++);
    m_insertFeedDataStmt.BindText(feedData.datetime, col++);
    m_insertFeedDataStmt.BindInt64(feedData.timestamp, col++);
    m_insertFeedDataStmt.BindInt64(feedData.createdtime, col++);
    m_insertFeedDataStmt.BindInt64(feedData.read, col++);
    m_insertFeedDataStmt.BindText(feedData.tag, col++);
    m_insertFeedDataStmt.BindText(feedData.misc, col++);
    m_insertFeedDataStmt.BindText(feedData.guid, col++);
    int ret = m_insertFeedDataStmt.Step();

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
    if (SQLITE_OK == (ret = m_queryFeedStmt.BindInt64(feedId, 1)))
    {
        Feed feed;
        ret = m_queryFeedStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeed(feed, stmt);
            fnQueryFeed(feed);
            });
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
    if (SQLITE_OK == (ret = m_queryFeedByGuidStmt.BindText(guid, 1)))
    {
        Feed feed;
        ret = m_queryFeedByGuidStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeed(feed, stmt);
            fnQueryFeed(feed);
            });
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
    int ret = m_queryAllFeedsStmt.Query([&](sqlite3_stmt* stmt) {
        QueryFeed(feed, stmt);
        fnQueryFeed(feed);
        });

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryAllFeedsOrderByTitleASC(FN_QUERY_FEED fnQueryFeed)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryAllFeedsOrderByTitleASCStmt.m_handle
        || !fnQueryFeed)
        return false;

    Feed feed;
    int ret = m_queryAllFeedsOrderByTitleASCStmt.Query([&](sqlite3_stmt* stmt) {
        QueryFeed(feed, stmt);
        fnQueryFeed(feed);
        });

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryAllFeedsOrderByTitleDESC(FN_QUERY_FEED fnQueryFeed)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryAllFeedsOrderByTitleDESCStmt.m_handle
        || !fnQueryFeed)
        return false;

    Feed feed;
    int ret = m_queryAllFeedsOrderByTitleDESCStmt.Query([&](sqlite3_stmt* stmt) {
        QueryFeed(feed, stmt);
        fnQueryFeed(feed);
        });

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataByFeedId(long long feedid, FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataByFeedIdStmt.m_handle
        || !fnQueryFeedData)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = m_queryFeedDataByFeedIdStmt.BindInt64(feedid, 1)))
    {
        FeedData feedData;
        ret = m_queryFeedDataByFeedIdStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeedData(feedData, stmt);
            fnQueryFeedData(feedData);
            });
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
    if (SQLITE_OK == (ret = m_queryFeedDataByFeedIdOrderByTimestampStmt.BindInt64(feedid, 1)))
    {
        FeedData feedData;
        ret = m_queryFeedDataByFeedIdOrderByTimestampStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeedData(feedData, stmt);
            fnQueryFeedData(feedData);
            });
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
    if (SQLITE_OK == (ret = m_queryFeedDataByOffsetOrderByTimestampStmt.BindInt64(offset, 1)))
    {
        FeedData feedData;
        ret = m_queryFeedDataByOffsetOrderByTimestampStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeedData(feedData, stmt);
            fnQueryFeedData(feedData);
            });
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
    if (SQLITE_OK == (ret = m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.BindInt64(feedid, 1))
        && SQLITE_OK == (ret = m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.BindInt64(offset, 2)))
    {
        FeedData feedData;
        ret = m_queryFeedDataByFeedIdByOffsetOrderByTimestampStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeedData(feedData, stmt);
            fnQueryFeedData(feedData);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryAllFeedDataOrderByTimestamp(FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryAllFeedDataOrderByTimestampStmt.m_handle
        || !fnQueryFeedData)
        return false;

    FeedData feedData;
    int ret = m_queryAllFeedDataOrderByTimestampStmt.Query([&](sqlite3_stmt* stmt) {
        QueryFeedData(feedData, stmt);
        fnQueryFeedData(feedData);
        });

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataByFeedDataId(long long feeddataid, FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataByFeedDataIdStmt.m_handle
        || !fnQueryFeedData)
        return false;

    int ret = SQLITE_OK;;
    if (SQLITE_OK == (ret = m_queryFeedDataByFeedDataIdStmt.BindInt64(feeddataid, 1)))
    {
        FeedData feedData;
        ret = m_queryFeedDataByFeedDataIdStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeedData(feedData, stmt);
            fnQueryFeedData(feedData);
            });
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
    if (SQLITE_OK == (ret = m_deleteOutdatedFeedDataStmt.BindInt64(timestamp, 1)))
        ret = m_deleteOutdatedFeedDataStmt.Step();

    return SQLITE_DONE == ret;
}

bool FeedDatabase::DeleteFeedByFeedId(long long feedid)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_deleteFeedByFeedIdStmt.m_handle)
        return false;

    int ret = SQLITE_OK;;
    if (SQLITE_OK == (ret = m_deleteFeedByFeedIdStmt.BindInt64(feedid, 1)))
        ret = m_deleteFeedByFeedIdStmt.Step();

    return SQLITE_DONE == ret;
}

bool FeedDatabase::DeleteFeedDataByFeedId(long long feedid)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_deleteFeedDataByFeedIdStmt.m_handle)
        return false;

    int ret = SQLITE_OK;;
    if (SQLITE_OK == (ret = m_deleteFeedDataByFeedIdStmt.BindInt64(feedid, 1)))
        ret = m_deleteFeedDataByFeedIdStmt.Step();

    return SQLITE_DONE == ret;
}

bool FeedDatabase::UpdateFeedDataReadColumn(long long feeddataid, long long read)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_updateFeedDataReadStmt.m_handle)
        return false;

    int ret = SQLITE_OK;;
    if (SQLITE_OK == (ret = m_updateFeedDataReadStmt.BindInt64(read, 1))
        && SQLITE_OK == (ret = m_updateFeedDataReadStmt.BindInt64(feeddataid, 2)))
    {
        ret = m_updateFeedDataReadStmt.Step();
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedTableDataExist(long long& exist)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedTableDataExistStmt.m_handle)
        return false;

    int ret = m_queryFeedTableDataExistStmt.Query([&](sqlite3_stmt* stmt) {
        exist = sqlite3_column_int64(stmt, 0);
        });

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataTableCount(long long& count)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataTableCountStmt.m_handle)
        return false;

    int ret = m_queryFeedDataTableCountStmt.Query([&](sqlite3_stmt* stmt) {
        count = sqlite3_column_int64(stmt, 0);
        });

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataTableCountByFeedId(long long feedid, long long& count)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataTableCountByFeedIdStmt.m_handle)
        return false;

    int ret = SQLITE_OK;;
    if (SQLITE_OK == (ret = m_queryFeedDataTableCountByFeedIdStmt.BindInt64(feedid, 1)))
    {
        ret = m_queryFeedDataTableCountByFeedIdStmt.Query([&](sqlite3_stmt* stmt) {
            count = sqlite3_column_int64(stmt, 0);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedTableCount(long long& count)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedTableCountStmt.m_handle)
        return false;

    int ret = m_queryFeedTableCountStmt.Query([&](sqlite3_stmt* stmt) {
        count = sqlite3_column_int64(stmt, 0);
        });

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedByOffset(long long offset, FN_QUERY_FEED fnQueryFeed)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedByOffsetStmt.m_handle
        || !fnQueryFeed)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = m_queryFeedByOffsetStmt.BindInt64(offset, 1)))
    {
        Feed feed;
        ret = m_queryFeedByOffsetStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeed(feed, stmt);
            fnQueryFeed(feed);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedByOffsetInRange(long long limit, long long offset, FN_QUERY_FEED fnQueryFeed)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedByOffsetInRangeStmt.m_handle
        || !fnQueryFeed)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = m_queryFeedByOffsetInRangeStmt.BindInt64(limit, 1))
        && SQLITE_OK == (ret = m_queryFeedByOffsetInRangeStmt.BindInt64(offset, 2)))
    {
        Feed feed;
        ret = m_queryFeedByOffsetInRangeStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeed(feed, stmt);
            fnQueryFeed(feed);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedByOffsetOrderByTitleASCInRange(long long limit, long long offset, FN_QUERY_FEED fnQueryFeed)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedOrderByTitleASCInRangeStmt.m_handle
        || !fnQueryFeed)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = m_queryFeedOrderByTitleASCInRangeStmt.BindInt64(limit, 1))
        && SQLITE_OK == (ret = m_queryFeedOrderByTitleASCInRangeStmt.BindInt64(offset, 2)))
    {
        Feed feed;
        ret = m_queryFeedOrderByTitleASCInRangeStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeed(feed, stmt);
            fnQueryFeed(feed);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedByOffsetOrderByTitleDESCInRange(long long limit, long long offset, FN_QUERY_FEED fnQueryFeed)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedOrderByTitleDESCInRangeStmt.m_handle
        || !fnQueryFeed)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = m_queryFeedOrderByTitleDESCInRangeStmt.BindInt64(limit, 1))
        && SQLITE_OK == (ret = m_queryFeedOrderByTitleDESCInRangeStmt.BindInt64(offset, 2)))
    {
        Feed feed;
        ret = m_queryFeedOrderByTitleDESCInRangeStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeed(feed, stmt);
            fnQueryFeed(feed);
            });
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
    if (SQLITE_OK == (ret = m_queryFeedDataByOffsetStmt.BindInt64(offset, 1)))
    {
        FeedData feedData;
        ret = m_queryFeedDataByOffsetStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeedData(feedData, stmt);
            fnQueryFeedData(feedData);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataOrderByTimestampInRange(long long limit, long long offset, FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataOrderByTimestampInRangeStmt.m_handle
        || !fnQueryFeedData)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = m_queryFeedDataOrderByTimestampInRangeStmt.BindInt64(limit, 1))
        && SQLITE_OK == (ret = m_queryFeedDataOrderByTimestampInRangeStmt.BindInt64(offset, 2)))
    {
        FeedData feedData;
        ret = m_queryFeedDataOrderByTimestampInRangeStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeedData(feedData, stmt);
            fnQueryFeedData(feedData);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataByFeedIdOrderByTimestampInRange(long long feedid, long long limit, long long offset, FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataByFeedIdOrderByTimestampInRangeStmt.m_handle
        || !fnQueryFeedData)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = m_queryFeedDataByFeedIdOrderByTimestampInRangeStmt.BindInt64(feedid, 1))
        && SQLITE_OK == (ret = m_queryFeedDataByFeedIdOrderByTimestampInRangeStmt.BindInt64(limit, 2))
        && SQLITE_OK == (ret = m_queryFeedDataByFeedIdOrderByTimestampInRangeStmt.BindInt64(offset, 3)))
    {
        FeedData feedData;
        ret = m_queryFeedDataByFeedIdOrderByTimestampInRangeStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeedData(feedData, stmt);
            fnQueryFeedData(feedData);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedExistByGuid(const std::string& guid, long long& exist)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedExistByGuidStmt.m_handle)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = m_queryFeedExistByGuidStmt.BindText(guid, 1)))
    {
        ret = m_queryFeedExistByGuidStmt.Query([&](sqlite3_stmt* stmt) {
            exist = sqlite3_column_int64(stmt, 0);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataCountByTitle(const std::string& title, long long& count)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataCountByTitle.m_handle)
        return false;

    int ret = SQLITE_OK;;
    if (SQLITE_OK == (ret = m_queryFeedDataCountByTitle.BindText(title, 1)))
    {
        ret = m_queryFeedDataCountByTitle.Query([&](sqlite3_stmt* stmt) {
            count = sqlite3_column_int64(stmt, 0);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataByTitleOrderByTimestampInRange(const std::string& title, long long limit, long long offset,
    FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataByTitleOrderByTimestampInRangeStmt.m_handle
        || !fnQueryFeedData)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = m_queryFeedDataByTitleOrderByTimestampInRangeStmt.BindText(title, 1))
        && SQLITE_OK == (ret = m_queryFeedDataByTitleOrderByTimestampInRangeStmt.BindInt64(limit, 2))
        && SQLITE_OK == (ret = m_queryFeedDataByTitleOrderByTimestampInRangeStmt.BindInt64(offset, 3)))
    {
        FeedData feedData;
        ret = m_queryFeedDataByTitleOrderByTimestampInRangeStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeedData(feedData, stmt);
            fnQueryFeedData(feedData);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataCountByFeedIdByTitle(long long feedid, const std::string& title, long long& count)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataCountByFeedIdByTitle.m_handle)
        return false;

    int ret = SQLITE_OK;;
    if (SQLITE_OK == (ret = m_queryFeedDataCountByFeedIdByTitle.BindInt64(feedid, 1))
        && SQLITE_OK == (ret = m_queryFeedDataCountByFeedIdByTitle.BindText(title, 2)))
    {
        ret = m_queryFeedDataCountByFeedIdByTitle.Query([&](sqlite3_stmt* stmt) {
            count = sqlite3_column_int64(stmt, 0);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedDataByFeedIdByTitleOrderByTimestampInRange(long long feedid, const std::string& title, long long limit, long long offset,
    FN_QUERY_FEED_DATA fnQueryFeedData)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedDataByFeedIdByTitleOrderByTimestampInRangeStmt.m_handle
        || !fnQueryFeedData)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = m_queryFeedDataByFeedIdByTitleOrderByTimestampInRangeStmt.BindInt64(feedid, 1))
        && SQLITE_OK == (ret = m_queryFeedDataByFeedIdByTitleOrderByTimestampInRangeStmt.BindText(title, 2))
        && SQLITE_OK == (ret = m_queryFeedDataByFeedIdByTitleOrderByTimestampInRangeStmt.BindInt64(limit, 3))
        && SQLITE_OK == (ret = m_queryFeedDataByFeedIdByTitleOrderByTimestampInRangeStmt.BindInt64(offset, 4)))
    {
        FeedData feedData;
        ret = m_queryFeedDataByFeedIdByTitleOrderByTimestampInRangeStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeedData(feedData, stmt);
            fnQueryFeedData(feedData);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedCountByTitle(const std::string& title, long long& count)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedCountByTitleStmt.m_handle)
        return false;

    int ret = SQLITE_OK;;
    if (SQLITE_OK == (ret = m_queryFeedCountByTitleStmt.BindText(title, 1)))
    {
        ret = m_queryFeedCountByTitleStmt.Query([&](sqlite3_stmt* stmt) {
            count = sqlite3_column_int64(stmt, 0);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedByTitleByOffsetInRange(const std::string& title, long long limit, long long offset, FN_QUERY_FEED fnQueryFeed)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedByTitleByOffsetInRangeStmt.m_handle
        || !fnQueryFeed)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = m_queryFeedByTitleByOffsetInRangeStmt.BindText(title, 1))
        && SQLITE_OK == (ret = m_queryFeedByTitleByOffsetInRangeStmt.BindInt64(limit, 2))
        && SQLITE_OK == (ret = m_queryFeedByTitleByOffsetInRangeStmt.BindInt64(offset, 3)))
    {
        Feed feed;
        ret = m_queryFeedByTitleByOffsetInRangeStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeed(feed, stmt);
            fnQueryFeed(feed);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedByTitleByOffsetOrderByTitleASCInRange(const std::string& title, long long limit, long long offset, FN_QUERY_FEED fnQueryFeed)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedByTitleByOffsetOrderByTitleASCInRangeStmt.m_handle
        || !fnQueryFeed)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = m_queryFeedByTitleByOffsetOrderByTitleASCInRangeStmt.BindText(title, 1))
        && SQLITE_OK == (ret = m_queryFeedByTitleByOffsetOrderByTitleASCInRangeStmt.BindInt64(limit, 2))
        && SQLITE_OK == (ret = m_queryFeedByTitleByOffsetOrderByTitleASCInRangeStmt.BindInt64(offset, 3)))
    {
        Feed feed;
        ret = m_queryFeedByTitleByOffsetOrderByTitleASCInRangeStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeed(feed, stmt);
            fnQueryFeed(feed);
            });
    }

    return SQLITE_DONE == ret;
}

bool FeedDatabase::QueryFeedByTitleByOffsetOrderByTitleDESCInRange(const std::string& title, long long limit, long long offset, FN_QUERY_FEED fnQueryFeed)
{
    ATL::CComCritSecLock lock(m_dbLock);
    if (!m_queryFeedByTitleByOffsetOrderByTitleDESCInRangeStmt.m_handle
        || !fnQueryFeed)
        return false;

    int ret = SQLITE_OK;
    if (SQLITE_OK == (ret = m_queryFeedByTitleByOffsetOrderByTitleDESCInRangeStmt.BindText(title, 1))
        && SQLITE_OK == (ret = m_queryFeedByTitleByOffsetOrderByTitleDESCInRangeStmt.BindInt64(limit, 2))
        && SQLITE_OK == (ret = m_queryFeedByTitleByOffsetOrderByTitleDESCInRangeStmt.BindInt64(offset, 3)))
    {
        Feed feed;
        ret = m_queryFeedByTitleByOffsetOrderByTitleDESCInRangeStmt.Query([&](sqlite3_stmt* stmt) {
            QueryFeed(feed, stmt);
            fnQueryFeed(feed);
            });
    }

    return SQLITE_DONE == ret;
}
