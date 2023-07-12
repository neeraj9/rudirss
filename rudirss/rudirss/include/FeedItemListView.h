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
    void InsertFeedItem(const FeedDatabase::FeedData& feedData);

    std::wstring GetReadStateSymbol(long long read);
    void MarkFeedDataAsReadOrUnRead(LPNMITEMACTIVATE activateItem, const std::wstring &title, long long read);

    virtual LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    const int GetWidth() const { return m_width; }
    const int GetHeight() const { return m_height; }

protected:
    int m_width;
    int m_height;
    int m_titleColumnWidth;
    int m_updatedColumnWidth;
};
