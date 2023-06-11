#pragma once

#include "WinHttpRequest.h"

#include <vector>
#include <functional>

class HttpGetRequest : public WinHttpRequest<HttpGetRequest>
{
public:
    using FN_ON_READ_COMPLETE = std::function<void(const char*, DWORD)>;
    using FN_ON_PROCESS_COMPLETE = std::function<void(const void*, DWORD)>;
    using FN_ON_EXCEPTION = std::function<void(const void*, const WINHTTP_ASYNC_RESULT *)>;

private:
    WinHttpConnection m_connection;
    bool m_result;
    DWORD m_statusCode;
    void* m_userParam;
    FN_ON_READ_COMPLETE m_onReadComplete;
    FN_ON_PROCESS_COMPLETE m_onProcessComplete;
    FN_ON_EXCEPTION m_onException;

protected:
    HRESULT GetStatusCode(DWORD& statusCode)
    {
        DWORD statusCodeSize = sizeof(DWORD);
        HRESULT result = E_FAIL;
        if (!::WinHttpQueryHeaders(m_handle,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &statusCode,
            &statusCodeSize,
            WINHTTP_NO_HEADER_INDEX))
        {
            result = HRESULT_FROM_WIN32(::GetLastError());
        }

        return result;
    }

    virtual void OnException(const WINHTTP_ASYNC_RESULT *asyncResult)
    {
        if (m_onException)
            m_onException(m_userParam, asyncResult);
    }

    virtual void OnReadComplete(const void* buffer, DWORD bytesRead)
    {
        //m_data.insert(m_data.end(), reinterpret_cast<const char*>(buffer), reinterpret_cast<const char*>(buffer) + bytesRead);
        if (m_onReadComplete)
            m_onReadComplete(reinterpret_cast<const char*>(buffer), bytesRead);
    }

    virtual void OnProcessComplete()
    {
        if (m_onProcessComplete)
            m_onProcessComplete(m_userParam, m_statusCode);
    }

    virtual void OnResponseComplete(HRESULT result)
    {
        Close();
        m_result = S_OK == result;
    }

    virtual HRESULT OnCallback(DWORD code, const void* info, DWORD length)
    {
        switch (code)
        {
        case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
        {
            if (!::WinHttpReceiveResponse(m_handle, 0))
            {
                return HRESULT_FROM_WIN32(::GetLastError());
            }

            break;
        }

        case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
        {
            m_statusCode = 0;
            if (FAILED(GetStatusCode(m_statusCode))
                || HTTP_STATUS_OK != m_statusCode)
            {
                return E_FAIL;
            }

            if (!::WinHttpReadData(m_handle, m_buffer.data(), m_buffer.size(), nullptr))
            {
                return HRESULT_FROM_WIN32(::GetLastError());
            }

            break;
        }

        case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
        {
            if (length > 0)
            {
                OnReadComplete(m_buffer.data(), length);

                if (!::WinHttpReadData(m_handle, m_buffer.data(), m_buffer.size(), nullptr))
                {
                    return HRESULT_FROM_WIN32(::GetLastError());
                }
            }
            else
            {
                OnResponseComplete(S_OK);
            }

            break;
        }

        case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
        {
            OnException(static_cast<WINHTTP_ASYNC_RESULT*>(const_cast<void*>(info)));

            return E_FAIL;
        }

        case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:
        {
            OnProcessComplete();
            break;
        }

        default:
            break;
        }

        return S_OK;
    }

public:
    HttpGetRequest() : m_result{ false }, m_statusCode{ 0 }, m_userParam{ nullptr }{ }

    virtual ~HttpGetRequest() { }

    virtual bool Initialize(const std::wstring& source, const WinHttpSession& session)
    {
        HRESULT result = E_FAIL;
        do
        {
            WCHAR host[BUFSIZ]{};
            URL_COMPONENTS urlComponents{};
            urlComponents.dwStructSize = sizeof(urlComponents);
            urlComponents.lpszHostName = host;
            urlComponents.dwHostNameLength = _countof(host);
            urlComponents.dwUrlPathLength = -1;
            if (!WinHttpCrackUrl(source.c_str(), 0, 0, &urlComponents))
            {
                result = HRESULT_FROM_WIN32(::GetLastError());
                break;
            }

            result = m_connection.Initialize(urlComponents.lpszHostName, urlComponents.nPort, session);
            if (FAILED(result))
                break;

            result = WinHttpRequest<HttpGetRequest>::Initialize(urlComponents.lpszUrlPath, 0, m_connection);
            if (FAILED(result))
                break;

        } while (0);

        return SUCCEEDED(result);
    }

    virtual bool SendRequest(const void* userParam, FN_ON_READ_COMPLETE onReadComplete,
        FN_ON_PROCESS_COMPLETE onProcessComplete, FN_ON_EXCEPTION onException)
    {
        m_userParam = const_cast<void *>(userParam);
        m_onReadComplete = onReadComplete;
        m_onProcessComplete = onProcessComplete;
        m_onException = onException;
        return SUCCEEDED(WinHttpRequest<HttpGetRequest>::SendRequest(nullptr, 0, nullptr, 0, 0));
    }
};
