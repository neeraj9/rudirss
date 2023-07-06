#include "Timer.h"

Timer::Timer() : m_waitEvent{ nullptr }, m_timer{ nullptr }
{
}

Timer::~Timer()
{
    Delete();
}

bool Timer::Create(WAITORTIMERCALLBACK Callback, PVOID parameter, DWORD dueTime, DWORD period, ULONG flags)
{
    if (!m_waitEvent)
    {
        m_waitEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    }

    return !!CreateTimerQueueTimer(&m_timer, nullptr, Callback, parameter, dueTime, period, flags);
}
void Timer::Delete()
{
    if (m_timer 
        && DeleteTimerQueueTimer(nullptr, m_timer, m_waitEvent) 
        && m_waitEvent)
    {
        WaitForSingleObject(m_waitEvent, INFINITE);
        CloseHandle(m_waitEvent);
        m_timer = nullptr;
        m_waitEvent = nullptr;
    }
}
