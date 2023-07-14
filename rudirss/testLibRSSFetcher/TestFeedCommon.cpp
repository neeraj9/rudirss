#include "pch.h"
#include "FeedCommon.h"
#include "FeedBase.h"
#include "FeedFetcher.h"

#include <format>
#include <fstream>
#include <atltime.h>

using namespace FeedCommon;

TEST(TestFeedCommon, TestIterateSiblingElements)
{
    ASSERT_TRUE(FeedCommon::Initialize());

    {
        try
        {
            WCHAR dir[256]{};
            GetCurrentDirectory(_countof(dir), dir);

            WinMSXML xml;
            ASSERT_TRUE(xml.Init());
            xml.Load(std::format(L"{}\\..\\..\\testdata\\sample-rss.xml", dir));
            ASSERT_TRUE(true);

            auto elements = xml.GetElementsByTagName(L"channel");
            ASSERT_TRUE(elements);

            long length = 0;
            elements->get_length(&length);
            elements->reset();

            WinMSXML::XMLElement channel;
            ASSERT_TRUE(SUCCEEDED(elements->get_item(0, &channel)));

            WinMSXML::XMLElement childElement;
            ASSERT_TRUE(SUCCEEDED(channel->get_firstChild(&childElement)));

            IterateSiblingElements(childElement, [](const std::wstring_view& name, const std::wstring_view& value,
                const WinMSXML::XMLElement& element) -> bool {
                    std::wcout << std::format(L"name: {}, value: {}\n", name, value);
                    return true;
                });
            ASSERT_TRUE(true);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
            ASSERT_FALSE(true);
        }
    }

    FeedCommon::Uninitialize();
}

TEST(TestFeedCommon, TestGetFeedSpecification)
{
    ASSERT_TRUE(FeedCommon::Initialize());

    {
        try
        {
            WCHAR dir[256]{};
            GetCurrentDirectory(_countof(dir), dir);

            WinMSXML xml;
            ASSERT_TRUE(xml.Init());
            xml.Load(std::format(L"{}\\..\\..\\testdata\\sample-rss.xml", dir));
            auto spec = GetFeedSpecification(xml);
            ASSERT_TRUE(FeedSpecification::RSS == spec);
            xml.Load(std::format(L"{}\\..\\..\\testdata\\hacker_news.xml", dir));
            spec = GetFeedSpecification(xml);
            ASSERT_TRUE(FeedSpecification::RSS == spec);
            xml.Load(std::format(L"{}\\..\\..\\testdata\\planet_emacs.xml", dir));
            spec = GetFeedSpecification(xml);
            ASSERT_TRUE(FeedSpecification::Atom == spec);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
            ASSERT_FALSE(true);
        }
    }

    FeedCommon::Uninitialize();
}

TEST(TestFeedCommon, TestCreateFeedBySpec)
{
    ASSERT_TRUE(FeedCommon::Initialize());

    auto fnLoadString = [](const std::wstring& file) -> std::wstring {
        std::wfstream f(file, std::iostream::in);
        std::wstring text;
        if (f)
        {
            f.seekg(f.beg, f.end);
            size_t size = f.tellg();
            f.seekg(0);
            text.resize(size, 0);
            f.read(const_cast<wchar_t*>(text.data()), size);
        }

        return text;
    };

    {
        try
        {
            WCHAR dir[256]{};
            GetCurrentDirectory(_countof(dir), dir);

            WinMSXML xml;
            ASSERT_TRUE(xml.Init());
            auto file = std::format(L"{}\\..\\..\\testdata\\sample-rss.xml", dir);
            xml.Load(file);
            auto spec = GetFeedSpecification(xml);
            ASSERT_TRUE(FeedSpecification::RSS == spec);

            auto xmlString = fnLoadString(file);
            auto rssFeed = CreateFeed(xmlString);
            FeedBase *rssFeedBase = reinterpret_cast<FeedBase *>(rssFeed.get());
            ASSERT_TRUE(FeedSpecification::RSS == rssFeedBase->GetSpec());

            xml.Load(std::format(L"{}\\..\\..\\testdata\\planet_emacs.xml", dir));
            spec = GetFeedSpecification(xml);
            ASSERT_TRUE(FeedSpecification::Atom == spec);

            file = std::format(L"{}\\..\\..\\testdata\\planet_emacs.xml", dir);
            xmlString = fnLoadString(file);
            auto atomFeed = CreateFeed(xmlString);
            FeedBase *atomFeedBase = reinterpret_cast<FeedBase *>(atomFeed.get());
            ASSERT_TRUE(FeedSpecification::Atom == atomFeedBase->GetSpec());
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
            ASSERT_FALSE(true);
        }
    }

    FeedCommon::Uninitialize();
}

TEST(TestFeedCommon, TestConvertWideStringToString)
{
    char dir[256]{};
    GetCurrentDirectoryA(_countof(dir), dir);
    auto file = std::format("{}\\..\\..\\testdata\\Soft_Job.xml", dir);

    std::fstream f(file, std::iostream::in);
    ASSERT_TRUE(f);
    f.seekg(f.beg, f.end);
    size_t size = f.tellg();
    f.seekg(0);
    std::string s;
    s.resize(size, 0);
    f.read(s.data(), size);

    std::wstring ws;
    bool result = ConvertStringToWideString(s, ws);
    ASSERT_TRUE(result);
}

TEST(TestFeedCommon, TestConvertStringToWideString)
{
    char dir[256]{};
    GetCurrentDirectoryA(_countof(dir), dir);
    auto file = std::format("{}\\..\\..\\testdata\\Soft_Job.xml", dir);

    std::fstream f(file, std::iostream::in);
    ASSERT_TRUE(f);
    f.seekg(f.beg, f.end);
    size_t size = f.tellg();
    f.seekg(0);
    std::string s;
    s.resize(size, 0);
    f.read(s.data(), size);

    std::wstring ws;
    bool result = ConvertStringToWideString(s, ws);
    ASSERT_TRUE(result);

    std::string sConvertBack;
    result = ConvertWideStringToString(ws, sConvertBack);
    ASSERT_TRUE(result);
}

TEST(TestFeedCommon, TestCreateFeedTask)
{
    try
    {
        FeedFetcher feedFetcher;
        std::string rawData;
        feedFetcher.Initialize([&](const FetchUnit* fetchUnit, bool result, DWORD statusCode, const char* data, size_t size,
            const WINHTTP_ASYNC_RESULT* asyncResult) {
                std::wcout << std::format(L"URL: {}, statusCode: {}\n", fetchUnit->FeedUrl(), statusCode);
                rawData.resize(size);
                std::copy(data, data + size, rawData.begin());
            });
        feedFetcher.StartFetchRoutine();

        feedFetcher.PushFeedUrl(L"https://www.ptt.cc/atom/Soft_Job.xml");

        Sleep(1000);
        feedFetcher.StopFetchRoutine();
        EXPECT_TRUE(0 == feedFetcher.GetFeedUrlSize());

        ASSERT_TRUE(!rawData.empty());
        auto feedTask = CreateFeedTask<FeedTask>(rawData.data(), rawData.size());
        ASSERT_TRUE(feedTask);
        DestroyFeedTask<FeedTask>(feedTask);
        ASSERT_TRUE(nullptr == feedTask.get());
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        ASSERT_FALSE(true);
    }
}

TEST(TestFeedCommon, TestGetUUID)
{
    std::wstring UUID = FeedCommon::GetUUID();
    ASSERT_TRUE(!UUID.empty());
}

TEST(TestFeedCommon, TestParseDatetime)
{
    std::string datetime = "2023-07-06T12:05:57+08:00";
    auto timestamp = FeedCommon::ConvertDatetimeToTimestamp(FeedCommon::FeedSpecification::Atom, datetime);
    CTime t = timestamp;
    ASSERT_TRUE(2023 == t.GetYear() && 7 == t.GetMonth() && 6 == t.GetDay()
        && 12 == t.GetHour() && 5 == t.GetMinute() && 57 == t.GetSecond());

    datetime = "Sun, 9 Jul 2023 07:57:07 +0000";
    timestamp = FeedCommon::ConvertDatetimeToTimestamp(FeedCommon::FeedSpecification::RSS, datetime);
    t = timestamp;
    ASSERT_TRUE(2023 == t.GetYear() && 7 == t.GetMonth() && 9 == t.GetDay()
        && 7 == t.GetHour() && 57 == t.GetMinute() && 7 == t.GetSecond());
}

TEST(TestFeedCommon, TestLoadOPML)
{
    ASSERT_TRUE(SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)));

    {
        WCHAR dir[256]{};
        GetCurrentDirectory(_countof(dir), dir);
        std::vector<std::wstring> feedUrls;
        FeedCommon::LoadFeedUrlsFromOPML(std::format(L"{}\\..\\..\\testdata\\subscriptionList.opml", dir), feedUrls);
        ASSERT_TRUE(!feedUrls.empty());
    }

    CoUninitialize();
}
