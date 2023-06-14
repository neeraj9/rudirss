#include "FetchUnit.h"

#include <stdexcept>

FetchUnit::FetchUnit() 
{

}

FetchUnit::FetchUnit(FetchUnit&& rhs) noexcept: HttpGetRequest(std::move(rhs)),
    m_data(rhs.m_data), m_onFetchComplete(rhs.m_onFetchComplete)
{
}

FetchUnit::~FetchUnit() 
{
}

void FetchUnit::Fetch(const WinHttpSession &session, const std::wstring& feedURL, 
    FN_ON_FETCH_COMPLETE onFetchComplete)
{
    if (!onFetchComplete)
        throw std::runtime_error("Error: invalid onFetchComplete callback function.");

    if (!Initialize(feedURL, session))
        throw std::runtime_error("Error: cannot initialize connection.");

    m_onFetchComplete = onFetchComplete;
    m_data.clear();
    auto result = SendRequest(nullptr, [&](const void* userParam, const char* data, DWORD size) {
        m_data.insert(m_data.end(), data, data + size);
        }, [&](const void* userParam, bool result, DWORD statusCode) {
            m_onFetchComplete(result, statusCode, m_data.data(), m_data.size(), nullptr);
        }, [&](const void* userParam, const WINHTTP_ASYNC_RESULT* asyncResult) {
            m_onFetchComplete(false, 0, nullptr, 0, asyncResult);
        });
    if (!result)
        throw std::runtime_error("Error: SendRequest failed.");
}
