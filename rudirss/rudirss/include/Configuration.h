#pragma once

#include <vector>
#include <string>

struct DatabaseConfiguration
{
    static const unsigned DEFAULT_RESERVE_DAYS = 3650;
    bool allowDeleteOutdatedFeedItems;
    unsigned reserveDays;
    DatabaseConfiguration() : allowDeleteOutdatedFeedItems{ true }, reserveDays{ DEFAULT_RESERVE_DAYS } {}
    DatabaseConfiguration(const DatabaseConfiguration& rhs) : allowDeleteOutdatedFeedItems{ rhs.allowDeleteOutdatedFeedItems },
        reserveDays{ rhs.reserveDays } {}
    DatabaseConfiguration(DatabaseConfiguration&& rhs) noexcept : allowDeleteOutdatedFeedItems{ rhs.allowDeleteOutdatedFeedItems },
        reserveDays{rhs.reserveDays} {}
    DatabaseConfiguration& operator=(const DatabaseConfiguration& rhs)
    {
        if (this != &rhs)
        {
            allowDeleteOutdatedFeedItems = rhs.allowDeleteOutdatedFeedItems;
            reserveDays = rhs.reserveDays;
        }

        return *this;
    }
};

struct DisplayConfiguration
{
    enum class FeedSortMethod
    {
        NONE,
        ASC,
        DESC,
    };

    int feedWidth;
    int feedItemTitleColumnWidth;
    int feedItemUpdatedColumnWidth;
    FeedSortMethod feedSortMethod;
    DisplayConfiguration() : feedWidth{ 0 }, feedItemTitleColumnWidth{ 0 }, feedItemUpdatedColumnWidth{ 0 }, feedSortMethod{ FeedSortMethod::ASC } {}
    DisplayConfiguration(const DisplayConfiguration& rhs) : feedWidth{ rhs.feedWidth },
        feedItemTitleColumnWidth{ rhs.feedItemTitleColumnWidth }, feedItemUpdatedColumnWidth{ rhs.feedItemUpdatedColumnWidth },
        feedSortMethod{ rhs.feedSortMethod } {}
    DisplayConfiguration(DisplayConfiguration&& rhs) noexcept : feedWidth{ rhs.feedWidth },
        feedItemTitleColumnWidth{ rhs.feedItemTitleColumnWidth }, feedItemUpdatedColumnWidth{ rhs.feedItemUpdatedColumnWidth },
        feedSortMethod{ rhs.feedSortMethod } {}
    DisplayConfiguration& operator=(const DisplayConfiguration& rhs)
    {
        if (this != &rhs)
        {
            feedWidth = rhs.feedWidth;
            feedItemTitleColumnWidth = rhs.feedItemTitleColumnWidth;
            feedItemUpdatedColumnWidth = rhs.feedItemUpdatedColumnWidth;
            feedSortMethod = rhs.feedSortMethod;
        }

        return *this;
    }
};
