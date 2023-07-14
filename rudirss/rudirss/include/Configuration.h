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
    DatabaseConfiguration& operator=(DatabaseConfiguration&& rhs) noexcept
    {
        if (this != &rhs)
        {
            allowDeleteOutdatedFeedItems = rhs.allowDeleteOutdatedFeedItems;
            reserveDays = rhs.reserveDays;
        }

        return *this;
    }
};

struct Configuration
{
    std::vector<std::wstring> feedUrls;
    DatabaseConfiguration dbConfiguration;
    Configuration() {}
    Configuration(const Configuration& rhs) : feedUrls{ rhs.feedUrls }, dbConfiguration{ rhs.dbConfiguration } {}
    Configuration(Configuration&& rhs) noexcept : feedUrls{ std::move(rhs.feedUrls) }, dbConfiguration{ std::move(rhs.dbConfiguration) } {}
    Configuration& operator=(const Configuration& rhs)
    {
        if (this != &rhs)
        {
            feedUrls = rhs.feedUrls;
            dbConfiguration = rhs.dbConfiguration;
        }

        return *this;
    }
    Configuration& operator=(Configuration&& rhs) noexcept
    {
        if (this != &rhs)
        {
            feedUrls = std::move(rhs.feedUrls);
            dbConfiguration = std::move(rhs.dbConfiguration);
        }

        return *this;
    }
};
