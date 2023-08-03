#pragma once

#include <winsqlite/winsqlite3.h>
#include <format>
#include <functional>

class SQLite3Stmt
{
public:
    sqlite3_stmt* m_handle;

    SQLite3Stmt() : m_handle{ nullptr } {}
    SQLite3Stmt(const SQLite3Stmt&) = delete;
    virtual ~SQLite3Stmt()
    {
        Close();
    }

    virtual void Close()
    {
        if (m_handle)
        {
            sqlite3_finalize(m_handle);
            m_handle= nullptr;
        }
    }

    void Prepare(sqlite3 *db, const std::string& stmt)
    {
        int ret = sqlite3_prepare_v2(db, stmt.c_str(), stmt.length(), &m_handle, nullptr);
        if (0 != ret)
            throw std::runtime_error(std::format("Error: cannot prepare '{}'.", stmt));
    }

    using FN_ON_QUERY_DATA = std::function<void(sqlite3_stmt* stmt)>;
    int Query(FN_ON_QUERY_DATA fnOnQueryData)
    {
        int ret = SQLITE_FAIL;
        while (SQLITE_ROW == (ret = sqlite3_step(m_handle)))
        {
            fnOnQueryData(m_handle);
        }
        sqlite3_reset(m_handle);

        return ret;
    }

    int Step()
    {
        int ret = sqlite3_step(m_handle);
        sqlite3_reset(m_handle);

        return ret;
    }

    int BindText(const std::string& text, int col)
    {
        return sqlite3_bind_text(m_handle, col, text.c_str(), text.length(), nullptr);
    }

    int BindInt64(long long val, int col)
    {
        return sqlite3_bind_int64(m_handle, col, val);
    }
};
