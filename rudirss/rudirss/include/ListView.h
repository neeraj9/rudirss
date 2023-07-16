#pragma once

#include "WindowHandle.h"

#include <functional>

class ListView : public WindowHandle
{
public:
    ListView() : m_width{ 0 }, m_height{ 0 } {}
    virtual ~ListView() {}

    using FN_PROCESS_MESSAGE = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>;
    virtual LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;

    const int GetWidth() const { return m_width; }
    const int GetHeight() const { return m_height; }

protected:
    FN_PROCESS_MESSAGE m_fnProcessMessage;
    int m_width;
    int m_height;
};
