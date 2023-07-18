#pragma once

#include "MainWindow.h"
#include "Viewer.h"
#include "RudiRSSClient.h"
#include "FeedDatabase.h"
#include "FeedListView.h"
#include "FeedItemListView.h"

#include <string>

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
    LRESULT OnCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void OnDestroy();

    void InittializeControl();
    void UpdateControl();
    void InitFont();
    void OpenImportOPMLDialog();
    void OpenImportListFileDialog();

    LRESULT OnProcessListViewCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    long long GetLastSelectedFeedDataId();
    bool GetLastSelectedFeedData(FeedDatabase::FeedData &feedData);

public:
    RudiRSSMainWindow();
    virtual ~RudiRSSMainWindow();

    virtual bool Initialize(HINSTANCE hInstance);

    FeedListView& GetFeedListView() { return m_feedListView; }
    FeedItemListView& GetFeedItemListView() { return m_feedItemListView; }
    Viewer& GetViewer() { return m_viewer; }
    RudiRSSClient& GetRudiRSSClient() { return m_rudiRSSClient; }
    BOOL IsViewerInitialized();

    const HINSTANCE GetHInstance() const { return m_hInstance; }
};
