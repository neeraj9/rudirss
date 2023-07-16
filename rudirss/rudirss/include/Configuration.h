#pragma once

#include <vector>
#include <string>

struct DatabaseConfiguration
{
    static const unsigned DEFAULT_RESERVE_DAYS = 365;
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
    int feedWidth;
    int feedItemTitleColumnWidth;
    int feedItemUpdatedColumnWidth;
    DisplayConfiguration() : feedWidth{ 0 }, feedItemTitleColumnWidth{ 0 }, feedItemUpdatedColumnWidth{ 0 } {}
    DisplayConfiguration(const DisplayConfiguration& rhs) : feedWidth{ rhs.feedWidth },
        feedItemTitleColumnWidth{ rhs.feedItemTitleColumnWidth }, feedItemUpdatedColumnWidth{ rhs.feedItemUpdatedColumnWidth } {}
    DisplayConfiguration(DisplayConfiguration&& rhs) noexcept : feedWidth{ rhs.feedWidth },
        feedItemTitleColumnWidth{ rhs.feedItemTitleColumnWidth }, feedItemUpdatedColumnWidth{ rhs.feedItemUpdatedColumnWidth } {}
    DisplayConfiguration& operator=(const DisplayConfiguration& rhs)
    {
        if (this != &rhs)
        {
            feedWidth = rhs.feedWidth;
            feedItemTitleColumnWidth = rhs.feedItemTitleColumnWidth;
            feedItemUpdatedColumnWidth = rhs.feedItemUpdatedColumnWidth;
        }

        return *this;
    }
};
