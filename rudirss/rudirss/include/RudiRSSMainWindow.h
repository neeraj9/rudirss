#pragma once

#include "MainWindow.h"
#include "Viewer.h"
#include "RudiRSSClient.h"
#include "FeedCommon.h"

#include <string>
#include <map>
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

    struct SimpleFeed
    {
        FeedData feedInfo;
        std::vector<FeedData> feedData;
        FeedCommon::FeedSpecification spec;
        SimpleFeed() : spec{FeedCommon::FeedSpecification::None} {}
        SimpleFeed(SimpleFeed&& rhs) noexcept: feedInfo(std::move(rhs.feedInfo)), feedData(std::move(rhs.feedData)),
            spec{ rhs.spec } {}
    };
    std::map<std::wstring, SimpleFeed> m_feeds;
    ATL::CComCriticalSection m_feedLock;

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
