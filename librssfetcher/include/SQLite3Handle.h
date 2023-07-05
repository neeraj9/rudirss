#pragma once

#include <winsqlite/winsqlite3.h>

class SQLite3Handle
{
public:
    sqlite3* m_handle;

    SQLite3Handle() : m_handle{ nullptr } {}
    SQLite3Handle(const SQLite3Handle&) = delete;
    virtual ~SQLite3Handle()
    {
        Close();
    }

    virtual void Close()
    {
        if (m_handle)
        {
            sqlite3_close_v2(m_handle);
            m_handle = nullptr;
        }
    }
};
