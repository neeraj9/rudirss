#pragma once

#include "MainWindow.h"
#include "Viewer.h"

#include <string>

class RudiRSSMainWindow : public MainWindow
{
protected:
    std::wstring m_title;
    std::wstring m_className;
    WindowHandle m_feedListBox;
    Viewer m_viewer;
    BOOL m_initViewer;

    virtual void OnRegister(WNDCLASSEXW& wcex);
    virtual HWND Create();
    virtual LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void InittializeControl();
    void UpdateControl();

    virtual LRESULT OnCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
    RudiRSSMainWindow();
    virtual ~RudiRSSMainWindow();

    virtual bool Initialize(HINSTANCE hInstance);
};
