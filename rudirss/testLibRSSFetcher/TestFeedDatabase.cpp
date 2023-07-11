#include "pch.h"
#include "FeedDatabase.h"
#include "FeedCommon.h"
#include <time.h>
#include <vector>

#include "FeedDatabase.h"


TEST(TestFeedDatabaseCase, TestDBOperations)
{
    try
    {
        FeedDatabase feedDatabase;
        feedDatabase.Open(L"testsql.db");
        feedDatabase.Initialize();
        ASSERT_TRUE(true);

        std::string sGUID = "d8c26708-ee30-44a0-8fd8-8b8fc20f4ae2";
        long long feedid = 1;
        std::string url = "https://news.ycombinator.com/rss";
        std::vector<std::string> links = { "https://www.nytimes.com/2023/06/16/us/daniel-ellsberg-dead.html", "https://propbox.co" };
        std::string datetime = "2023-06-12T13:33:08+08:00";
        std::string tag = "tech";
        std::string misc = "misc";
        std::string title = "title";
        time_t timestamp = 0;
        time_t createdtime = time(nullptr);

        FeedDatabase::Feed feed;
        feed.guid = sGUID;
        feed.url = url;
        feed.title = "test title";
        auto result = feedDatabase.InsertFeed(feed);
        ASSERT_TRUE(result);

        result = feedDatabase.QueryAllFeeds([](const FeedDatabase::Feed& feed) {
            });
        ASSERT_TRUE(result);

        for (const auto& link : links)
        {
            FeedDatabase::FeedData feedData;
            feedData.guid = link;
            feedData.feedid = feedid;
            feedData.link = link;
            feedData.title = title;
            feedData.datetime = datetime;
            feedData.timestamp = timestamp;
            feedData.createdtime = createdtime;
            feedData.tag = tag;
            feedData.misc = misc;
            feedDatabase.InsertFeedData(feedData);
        }

        result = feedDatabase.QueryFeedDataByFeedId(feedid, [](const FeedDatabase::FeedData& feedData) {
            });
        ASSERT_TRUE(result);

        result = feedDatabase.DeleteAllFeedData();
        ASSERT_TRUE(result);

        result = feedDatabase.DeleteAllFeeds();
        ASSERT_TRUE(result);
    }
    catch (const std::exception& e)
    {
        ASSERT_FALSE(true);
    }
}