#pragma once

#include "MainWindow.h"
#include "Viewer.h"
#include "RudiRSSClient.h"
#include "FeedDatabase.h"

#include <string>
#include <list>
#include <set>
#include <atlcore.h>
#include <CommCtrl.h>

class RudiRSSMainWindow : public MainWindow
{
protected:
    std::wstring m_title;
    std::wstring m_className;
    WindowHandle m_feedListBox;
    WindowHandle m_feedTitleListView;
    Viewer m_viewer;
    BOOL m_initViewer;
    RudiRSSClient m_rudiRSSClient;
    std::set<long long> m_feedIdSet;
    ATL::CComCriticalSection m_feedListLock;

    HFONT m_font;

    virtual void OnRegister(WNDCLASSEXW& wcex);
    virtual HWND Create();
    virtual LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void InittializeControl();
    void UpdateControl();
    void InitFont();
    void InitFeedListBox(int x, int y, int width, int height);
    void InitFeedTitleListView(int x, int y, int width, int height, int titleColWidth, int updatedColWidth);
    void InsertIntoFeedTitleListView(const FeedDatabase::FeedData& feedData);
    long long GetFeedIdFromFeedTitleListView();
    LPARAM GetLParamFromListView(LPNMITEMACTIVATE activateItem);

    virtual LRESULT OnListBoxCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void OnProcessFeedListBox(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnProcessListViewCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void OnProcessFeedTitleListView(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    bool FeedIdExistInSet(long long feedid);
    void InsertFeedIdIntoSet(long long feedid);
    void ClearFeedIdSet();

public:
    RudiRSSMainWindow();
    virtual ~RudiRSSMainWindow();

    virtual bool Initialize(HINSTANCE hInstance);
};
