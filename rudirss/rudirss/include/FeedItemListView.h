#pragma once

#include "ListView.h"
#include "FeedDatabase.h"
#include "ListViewCache.h"

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
    void DeleteAllItems();

    virtual void ClearCache();

    const int GetLastRightClickedItem() const { return m_lastRighClickedItem; }
    bool GetRightClickedFeedDataFromCache(FeedDatabase::FeedData& feedData);
    ListViewCache<FeedDatabase::FeedData>::iterator GetRightClickedFeedDataIteratorFromCache(bool &result);

protected:
    int m_titleColumnWidth;
    int m_updatedColumnWidth;
    int m_lastRighClickedItem;
    ListViewCache<FeedDatabase::FeedData> m_cache;
    RudiRSSMainWindow* m_mainWindow;

    void QueryFeedDataOrderByTimestampInRange(long long from, long long to);
    void QueryFeedDataByTitleOrderByTimestampInRange(long long from, long long to);
    void QueryFeedDataByFeedIdOrderByTimestampInRange(long long from, long long to);
    void QueryFeedDataByFeedIdByTitleOrderByTimestampInRange(long long from, long long to);
};
