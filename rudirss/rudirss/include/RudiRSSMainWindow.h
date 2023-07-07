#pragma once

#include "MainWindow.h"
#include "Viewer.h"
#include "RudiRSSClient.h"
#include "FeedCommon.h"

#include <string>
#include <list>
#include <atlcore.h>

class RudiRSSMainWindow : public MainWindow
{
protected:
    std::wstring m_title;
    std::wstring m_className;
    WindowHandle m_feedListBox;
    WindowHandle m_feedTitleListBox;
    Viewer m_viewer;
    BOOL m_initViewer;
    RudiRSSClient m_rudiRSSClient;

    virtual void OnRegister(WNDCLASSEXW& wcex);
    virtual HWND Create();
    virtual LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void InittializeControl();
    void UpdateControl();

    virtual LRESULT OnCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void OnProcessFeedList(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void OnProcessFeedTitleList(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void EnableWindow(BOOL enable);

public:
    RudiRSSMainWindow();
    virtual ~RudiRSSMainWindow();

    virtual bool Initialize(HINSTANCE hInstance);
};
