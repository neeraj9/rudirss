#pragma once

#include <winsqlite/winsqlite3.h>

class SQLite3StmtHandle
{
public:
    sqlite3_stmt* m_handle;

    SQLite3StmtHandle() : m_handle{ nullptr } {}
    SQLite3StmtHandle(const SQLite3StmtHandle&) = delete;
    virtual ~SQLite3StmtHandle()
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
};
