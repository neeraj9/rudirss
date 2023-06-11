#pragma once

#include "WinHttpRequest.h"

#if 0
typedef struct Request
{
    WinHttpConnection connection;
    TestGetRequest testGetRequest;
    Request() {}
    Request(const Request& rhs) = delete;
    Request(Request&& rhs) noexcept
    {
        connection.m_handle = rhs.connection.m_handle;
        rhs.connection.m_handle = nullptr;
        testGetRequest.m_handle = rhs.testGetRequest.m_handle;
        rhs.testGetRequest.m_handle = nullptr;
    }
    void operator= (Request& rhs) = delete;
};
#endif

class HttpClient
{
protected:
    WinHttpConnection m_connection;
};
