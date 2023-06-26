#pragma once

#include <Windows.h>

class Task
{
public:
    virtual ~Task() = 0 {}

    virtual void DoTask(void *param, OVERLAPPED *overlapped) = 0;
};
