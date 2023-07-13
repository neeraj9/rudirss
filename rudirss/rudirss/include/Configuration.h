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

struct Configuration
{
    std::vector<std::wstring> feedUrls;
    TimerConfiguration timerConfiguration;
    Configuration() {}
    Configuration(const Configuration& rhs) : feedUrls{ rhs.feedUrls }, timerConfiguration{ rhs.timerConfiguration } {}
    Configuration(Configuration&& rhs) noexcept : feedUrls{ std::move(rhs.feedUrls) }, timerConfiguration{ std::move(rhs.timerConfiguration) } {}
    Configuration& operator=(Configuration&& rhs) noexcept
    {
        if (this != &rhs)
        {
            feedUrls = std::move(rhs.feedUrls);
            timerConfiguration = std::move(rhs.timerConfiguration);
        }

        return *this;
    }
};
