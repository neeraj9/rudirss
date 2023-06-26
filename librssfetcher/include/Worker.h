#pragma once

#include <Windows.h>

class Worker
{
public:
    using RequestType = DWORD_PTR;

    virtual ~Worker() = 0 {}

    virtual BOOL Initialize(void* param) = 0;
    virtual void Terminate(void* param) = 0;
    virtual void Execute(RequestType requestType, void* param, OVERLAPPED* overlapped) = 0;
};
