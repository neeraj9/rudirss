#include "pch.h"
#include "AtomFeed.h"

#include <format>

TEST(TestAtomFeed, TestLoadSampleFeed)
{
    ASSERT_TRUE(SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)));

    {
        try
        {
            WCHAR dir[256]{};
            GetCurrentDirectory(_countof(dir), dir);

            AtomFeed atomFeed;
            atomFeed.ParseFromFile(std::format(L"{}\\..\\..\\testdata\\planet_emacs.xml", dir));
            atomFeed.IterateFeeds([](const FeedData& feed) -> bool {
                wprintf(L"title: %s\nid: %s\ndescription: %s\n\n",
                feed.GetValue(L"title").c_str(), feed.GetValue(L"id").c_str(), feed.GetValue(L"content").c_str());
                return true;
            });
            ASSERT_TRUE(true);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
            ASSERT_FALSE(true);
        }

        try
        {
            AtomFeed atomFeed;
            atomFeed.ParseFromFile(L"wrong_feed.xml");
            ASSERT_FALSE(true);
        }
        catch (const std::exception& e)
        {
            ASSERT_TRUE(true);
        }
    }

    CoUninitialize();
}
