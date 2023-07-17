#pragma once

#include "ListView.h"
#include "FeedDatabase.h"

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

protected:
    int m_titleColumnWidth;
    int m_updatedColumnWidth;

    RudiRSSMainWindow* m_mainWindow;
};
