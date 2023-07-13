#pragma once

#include "ListView.h"
#include "FeedDatabase.h"

#include <set>
#include <functional>
#include <atlcore.h>

class FeedListView : public ListView
{
public:
    FeedListView();
    virtual ~FeedListView();

    void Initialize(HWND hWnd, HINSTANCE hInstance, HMENU windowId, int x, int y, int width, int height,
        FN_PROCESS_MESSAGE fnProcessMessage);
    void InsertFeed(const FeedDatabase::Feed& feed);

    bool FeedIdExistInSet(long long feedid);
    void InsertFeedIdIntoSet(long long feedid);
    void ClearFeedIdSet();

    virtual LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
    std::set<long long> m_feedIdSet;
    ATL::CComCriticalSection m_lock;
};
