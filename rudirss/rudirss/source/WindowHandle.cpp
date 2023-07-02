#include "WindowHandle.h"

WindowHandle::WindowHandle() : m_hWnd{ nullptr }
{
}

WindowHandle::~WindowHandle()
{
    Destroy();
}

bool WindowHandle::Attach(HWND hWnd)
{
    m_hWnd = hWnd;
    return nullptr != m_hWnd;
}

HWND WindowHandle::Detach()
{
    HWND hWnd = m_hWnd;
    m_hWnd = nullptr;
    return hWnd;
}
void WindowHandle::Destroy()
{
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}


