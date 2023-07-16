#pragma once

#include "ListView.h"
#include "FeedDatabase.h"

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

    virtual LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
    ATL::CComCriticalSection m_lock;
};
