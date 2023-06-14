#include "pch.h"

#include "HttpGetRequest.h"

#include <Windows.h>
#include <vector>
#include <iostream>

class TestHttpGetRequest : public HttpGetRequest
{
public:
    virtual ~TestHttpGetRequest() {}
};

TEST(TestCaseName, TestName)
{
    WinHttpSession session;
    ASSERT_TRUE(SUCCEEDED(session.Initialize()));

    TestHttpGetRequest testHttpGetRequest;
    ASSERT_TRUE(SUCCEEDED(testHttpGetRequest.Initialize(L"https://learn.microsoft.com/en-us/", session)));

    HANDLE waitEvent = nullptr;
    waitEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    ASSERT_TRUE(nullptr != waitEvent);
    std::vector<char> responseData;
    DWORD resultStatusCode = 0;
    testHttpGetRequest.SendRequest(nullptr,
        [&](const void* userParam, const char* data, DWORD size) {
            responseData.insert(responseData.end(), data, data + size);
        }, [&](const void* userParam, bool result, DWORD statusCode) {
            std::cout << "Get server response, status code: " << statusCode << std::endl;
            resultStatusCode = statusCode;
            SetEvent(waitEvent);
        }, [&](const void* userParam, const WINHTTP_ASYNC_RESULT* asyncResult) {
            std::cout << "Error: " << asyncResult->dwError << std::endl;
            SetEvent(waitEvent);
        });

    WaitForSingleObject(waitEvent, INFINITE);
    CloseHandle(waitEvent);

    EXPECT_TRUE(HTTP_STATUS_OK == resultStatusCode);
    EXPECT_TRUE(!responseData.empty());
}