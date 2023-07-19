#pragma once

#include "ListView.h"
#include "FeedDatabase.h"
#include "ListViewCache.h"

#include <vector>

class RudiRSSMainWindow;

class FeedListView : public ListView
{
public:
    FeedListView();
    explicit FeedListView(RudiRSSMainWindow *mainWindow);
    virtual ~FeedListView();

    void Initialize(HWND hWnd, HINSTANCE hInstance, HMENU windowId, int x, int y, int width, int height);

    virtual LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    static const int ALL_FEEDS_LIST_INDEX = 0;

    long long GetLastSelectedFeedId();
    void ResetLastSelectedFeedId();
    int GetLastSelectedFeedIndex();
    void ResetLastSelectedFeedIndex();

    void UpdateFeedListFromDatabase();
    void UpdateFeedList(const std::vector<std::wstring> &feedUrls);

    void DeleteAllItems();
    virtual void ClearCache();

    const int GetLastRightClickedItem() const { return m_lastRighClickedItem; }

protected:
    long long m_lastSelectedFeedId;
    int m_lastSelectedFeedIndex;
    int m_lastRighClickedItem;
    RudiRSSMainWindow* m_mainWindow;
    ListViewCache<FeedDatabase::Feed> m_cache;
};
