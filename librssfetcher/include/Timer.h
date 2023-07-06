#pragma once

#include <Windows.h>

class Timer
{
protected:
    HANDLE m_waitEvent;
    HANDLE m_timer;

public:
    Timer();
    Timer(const Timer&) = delete;
    virtual ~Timer();

    bool Create(WAITORTIMERCALLBACK Callback, PVOID parameter, DWORD dueTime, DWORD period, ULONG flags);
    void Delete();
};
