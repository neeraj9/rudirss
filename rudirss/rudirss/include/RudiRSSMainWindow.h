#pragma once

#include "MainWindow.h"
#include "Viewer.h"
#include "RudiRSSClient.h"
#include "FeedDatabase.h"
#include "FeedListView.h"
#include "FeedItemListView.h"

#include <string>
#include <set>
#include <atlcore.h>
#include <CommCtrl.h>

class RudiRSSMainWindow : public MainWindow
{
protected:
    std::wstring m_title;
    std::wstring m_className;
    FeedListView m_feedListView;
    FeedItemListView m_feedItemListView;
    Viewer m_viewer;
    BOOL m_initViewer;
    RudiRSSClient m_rudiRSSClient;

    HFONT m_font;

    virtual void OnRegister(WNDCLASSEXW& wcex);
    virtual HWND Create();
    LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void OnDestroy();

    void InittializeControl();
    void UpdateControl();
    void InitFont();

    LRESULT OnProcessListViewCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
    RudiRSSMainWindow();
    virtual ~RudiRSSMainWindow();

    virtual bool Initialize(HINSTANCE hInstance);
};
