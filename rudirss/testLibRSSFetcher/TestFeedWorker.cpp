#include "pch.h"

#include "FeedClient.h"
#include "FeedCommon.h"
#include "FeedBase.h"

#include <fstream>
#include <format>

class FakeFeedClient : public FeedClient
{
public:
    FakeFeedClient() {}
    virtual ~FakeFeedClient() {}

    virtual void OnFeedReady(const std::unique_ptr<Feed>& feed)
    {
        if (!feed)
            return;

        char tmpPath[MAX_PATH]{};
        DWORD len = GetTempPathA(_countof(tmpPath), tmpPath);
        char tmpFile[MAX_PATH]{};
        UINT ret = GetTempFileNameA(tmpPath, "t_", 0, tmpFile);
        std::fstream fs(tmpFile, std::iostream::out);
        if (fs)
        {
            FeedBase* feedBase = reinterpret_cast<FeedBase*>(feed.get());
            auto spec = feedBase->GetSpec();
            feed->IterateFeeds([&](const FeedData& feed) -> bool {
                if (FeedSpecification::RSS == spec)
                {
                    auto ws = std::format(L"Title: {}\nLink: {}\nDescription: {}\n\n",
                        feed.GetValue(L"title"), feed.GetValue(L"link"), feed.GetValue(L"description"));
                    std::string s;
                    auto result = FeedCommon::ConvertWideStringToString(ws, s);
                    fs.write(s.c_str(), s.length());
                }
                else if (FeedSpecification::Atom == spec)
                {
                    auto ws = std::format(L"Title: {}\nLink: {}\nDescription: {}\n\n",
                        feed.GetValue(L"title"), feed.GetValue(L"id"), feed.GetValue(L"content"));
                    std::string s;
                    auto result = FeedCommon::ConvertWideStringToString(ws, s);
                    fs.write(s.c_str(), s.length());
                }

                return true;
                });

            std::wcout << L"Created file: " << tmpFile << std::endl;
        }

    }
};

TEST(TestFeedWorkerCase, TestConsumeFeedTask)
{
    FeedCommon::Initialize();

    {
        FakeFeedClient feedClient;
        bool result = feedClient.Initialize();
        ASSERT_TRUE(result);

        std::vector<std::wstring> feeds = { L"https://www.majorgeeks.com/files/rss", L"https://www.geeksforgeeks.org//feed",
            L"https://news.ycombinator.com/rss", L"https://planet.emacslife.com/atom.xml", L"https://totalcmd.net/rss.xml",
            L"https://www.ptt.cc/atom/Tech_Job.xml", L"https://www.ptt.cc/atom/Soft_Job.xml", L"https://www.ptt.cc/atom/careerplan.xml",
            L"https://www.ptt.cc/atom/salary.xml", L"https://www.ptt.cc/atom/Job.xml" };
        for (const auto feed : feeds)
        {
            feedClient.ConsumeFeed(feed);
        }
        Sleep(1000);
        feedClient.Shutdown();
    }

    FeedCommon::Uninitialize();
}