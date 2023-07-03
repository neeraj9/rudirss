#pragma once

#include "MainWindow.h"

#include <string>

class RudiRSSMainWindow : public MainWindow
{
protected:
    std::wstring m_title;
    std::wstring m_className;

    virtual void OnRegister(WNDCLASSEXW& wcex);
    virtual HWND Create();
    virtual LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
    RudiRSSMainWindow();
    virtual ~RudiRSSMainWindow();
};
