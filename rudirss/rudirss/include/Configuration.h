#pragma once

#include <vector>
#include <string>

struct TimerConfiguration
{
    static const unsigned DEFAULT_DUETIME = 0;
    static const unsigned DEFAULT_PERIOD = 1800 * 1000;
    unsigned dueTime;
    unsigned period;
    TimerConfiguration() : dueTime{ DEFAULT_DUETIME }, period{ DEFAULT_PERIOD } {}
    TimerConfiguration(const TimerConfiguration& rhs) : dueTime{ rhs.dueTime }, period{ rhs.period } {}
    TimerConfiguration& operator=(TimerConfiguration&& rhs) noexcept
    {
        if (this != &rhs)
        {
            dueTime = rhs.dueTime;
            period = rhs.period;
        }

        return *this;
    }
};

struct DatabaseConfiguration
{
    static const unsigned DEFAULT_RESERVE_DAYS = 365;
    bool deleteOutdatedFeedItems;
    unsigned reserveDays;
    DatabaseConfiguration() : deleteOutdatedFeedItems{ true }, reserveDays{ DEFAULT_RESERVE_DAYS } {}
    DatabaseConfiguration(const DatabaseConfiguration& rhs) : deleteOutdatedFeedItems{ rhs.deleteOutdatedFeedItems },
        reserveDays{ rhs.reserveDays } {}
    DatabaseConfiguration(DatabaseConfiguration&& rhs) noexcept : deleteOutdatedFeedItems{ rhs.deleteOutdatedFeedItems }, 
        reserveDays{rhs.reserveDays} {}
    DatabaseConfiguration& operator=(DatabaseConfiguration&& rhs) noexcept
    {
        if (this != &rhs)
        {
            deleteOutdatedFeedItems = rhs.deleteOutdatedFeedItems;
            reserveDays = rhs.reserveDays;
        }

        return *this;
    }
};

struct Configuration
{
    std::vector<std::wstring> feedUrls;
    TimerConfiguration timerConfiguration;
    DatabaseConfiguration dbConfiguration;
    Configuration() {}
    Configuration(const Configuration& rhs) : feedUrls{ rhs.feedUrls }, timerConfiguration{ rhs.timerConfiguration },
        dbConfiguration{ rhs.dbConfiguration } {}
    Configuration(Configuration&& rhs) noexcept : feedUrls{ std::move(rhs.feedUrls) },
        timerConfiguration{ std::move(rhs.timerConfiguration) }, dbConfiguration{ std::move(rhs.dbConfiguration) } {}
    Configuration& operator=(Configuration&& rhs) noexcept
    {
        if (this != &rhs)
        {
            feedUrls = std::move(rhs.feedUrls);
            timerConfiguration = std::move(rhs.timerConfiguration);
            dbConfiguration = std::move(rhs.dbConfiguration);
        }

        return *this;
    }
};
