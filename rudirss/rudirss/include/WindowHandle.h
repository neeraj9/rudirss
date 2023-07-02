#pragma once

#include <Windows.h>

class WindowHandle
{
public:
    HWND m_hWnd;

    WindowHandle();
    virtual ~WindowHandle();

    bool Attach(HWND hWnd);
    HWND Detach();
    void Destroy();
};
