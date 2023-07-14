#pragma once

#include "Timer.h"
#include "FeedDatabase.h"
#include <string>

class RefreshTimer : public Timer
{
protected:
    std::wstring m_feedGuid;
    void* m_param;

public:
    RefreshTimer() : m_param{ nullptr } {}
    RefreshTimer(const RefreshTimer &) = delete;
    RefreshTimer(void* param, const std::wstring& feedGuid) :m_feedGuid{ feedGuid }, m_param{ param } {}
    RefreshTimer(RefreshTimer&& rhs) noexcept
    {
        m_timer = rhs.m_timer;
        rhs.m_timer = nullptr;
        m_waitEvent = rhs.m_waitEvent;
        rhs.m_waitEvent = nullptr;
        m_feedGuid = std::move(rhs.m_feedGuid);
        m_param = rhs.m_param;
    }
    RefreshTimer& operator=(const RefreshTimer&) = delete;
    virtual ~RefreshTimer() {}

    const std::wstring GetFeedGuid() const { return m_feedGuid; }
    void* GetParam() const { return m_param; }
};
