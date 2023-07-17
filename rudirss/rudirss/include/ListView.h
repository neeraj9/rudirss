#pragma once

#include "WindowHandle.h"

class ListView : public WindowHandle
{
public:
    ListView() : m_width{ 0 }, m_height{ 0 } {}
    virtual ~ListView() {}

    virtual LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
    virtual void ClearCache() = 0;

    const int GetWidth() const { return m_width; }
    const int GetHeight() const { return m_height; }

protected:
    int m_width;
    int m_height;
};
