#include "pch.h"

#include "FeedData.h"

TEST(TestFeedData, TestSetFeedData)
{
    FeedData feedData;
    feedData.SetValue(L"title", L"testTitle");
    ASSERT_TRUE(L"testTitle" == feedData.GetValue(L"title"));
    feedData.SetValue(L"description", L"A simple description.");
    ASSERT_TRUE(L"A simple description." == feedData.GetValue(L"description"));
}

TEST(TestFeedData, TestCopyeedData)
{
    FeedData feedData;
    feedData.SetValue(L"title", L"testTitle");

    {
        FeedData copy;
        copy = feedData;
        ASSERT_TRUE(L"testTitle" == copy.GetValue(L"title"));
    }

    {
        FeedData copy(feedData);
        ASSERT_TRUE(L"testTitle" == copy.GetValue(L"title"));
    }
}