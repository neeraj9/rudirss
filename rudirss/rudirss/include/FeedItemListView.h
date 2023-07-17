#pragma once

#include "ListView.h"
#include "FeedDatabase.h"

#include <map>

class RudiRSSMainWindow;

class FeedItemListView : public ListView
{
public:
    FeedItemListView();
    explicit FeedItemListView(RudiRSSMainWindow *mainWindow);
    virtual ~FeedItemListView();

    void Initialize(HWND hWnd, HINSTANCE hInstance, HMENU windowId, int x, int y, int width, int height,
        int titleColWidth, int updatedColWidth);

    virtual LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void UpdateSelectedFeed(long long feedid);
    void UpdateAllFeeds();

    virtual void ClearCache();

protected:
    int m_titleColumnWidth;
    int m_updatedColumnWidth;
    std::map<long long, FeedDatabase::FeedData> m_cache;
    RudiRSSMainWindow* m_mainWindow;
};
