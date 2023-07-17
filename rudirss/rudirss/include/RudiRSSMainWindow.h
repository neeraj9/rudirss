#pragma once

#include "MainWindow.h"
#include "Viewer.h"
#include "RudiRSSClient.h"
#include "FeedDatabase.h"
#include "FeedListView.h"
#include "FeedItemListView.h"

#include <string>
#include <atlcore.h>
#include <CommCtrl.h>

class RudiRSSMainWindow : public MainWindow
{
protected:
    static const int ALL_FEEDS_LIST_INDEX = 0;

    std::wstring m_title;
    std::wstring m_className;
    FeedListView m_feedListView;
    FeedItemListView m_feedItemListView;
    Viewer m_viewer;
    BOOL m_initViewer;
    RudiRSSClient m_rudiRSSClient;
    long long m_lastSelectedFeedId;
    int m_lastSelectedFeedIndex;

    HFONT m_font;

    virtual void OnRegister(WNDCLASSEXW& wcex);
    virtual HWND Create();
    LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void OnDestroy();

    void InittializeControl();
    void UpdateControl();
    void InitFont();
    void UpdateSelectedFeed(long long feedid);
    void UpdateAllFeeds();
    void OpenImportOPMLDialog();
    void OpenImportListFileDialog();

    LRESULT OnProcessListViewCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
    RudiRSSMainWindow();
    virtual ~RudiRSSMainWindow();

    virtual bool Initialize(HINSTANCE hInstance);
};
