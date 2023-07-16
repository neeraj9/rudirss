#pragma once

#include "ListView.h"
#include "FeedDatabase.h"

#include <functional>
#include <string>
#include <CommCtrl.h>

class FeedItemListView : public ListView
{
public:
    FeedItemListView();
    virtual ~FeedItemListView();

    void Initialize(HWND hWnd, HINSTANCE hInstance, HMENU windowId, int x, int y, int width, int height,
        int titleColWidth, int updatedColWidth, FN_PROCESS_MESSAGE fnProcessMessage);

    virtual LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
    int m_titleColumnWidth;
    int m_updatedColumnWidth;
};
