#pragma once

#include "HttpGetRequest.h"

#include <Windows.h>
#include <string>
#include <vector>
#include <functional>

class FetchUnit: public HttpGetRequest
{
public:
    FetchUnit();
    FetchUnit(const FetchUnit& rhs) = delete;
    FetchUnit(FetchUnit&& rhs) noexcept;
    virtual ~FetchUnit();

    using FN_ON_FETCH_COMPLETE = std::function<void(bool, DWORD, const char *, size_t, const WINHTTP_ASYNC_RESULT *)>;
    void Fetch(const WinHttpSession &session, const std::wstring &feedURL, FN_ON_FETCH_COMPLETE onFetchComplete);

private:
    std::vector<char> m_data;
    FN_ON_FETCH_COMPLETE m_onFetchComplete;
};
