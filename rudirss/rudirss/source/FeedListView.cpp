#include "FeedListView.h"
#include "FeedCommon.h"
#include "Resource.h"

#include <atlbase.h>
#include <CommCtrl.h>

FeedListView::FeedListView() : m_width{ 0 }, m_height{ 0 }
{
    m_lock.Init();
}

FeedListView::~FeedListView()
{

}

void FeedListView::Initialize(HWND hWnd, HINSTANCE hInstance, HMENU windowId, int x, int y, int width, int height,
    FN_PROCESS_MESSAGE fnProcessMessage)
{
    m_width = width;
    m_height = height;
    m_fnProcessMessage = fnProcessMessage;

    constexpr DWORD dwStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS;
    Attach(CreateWindowEx(WS_EX_STATICEDGE, WC_LISTVIEW, nullptr, dwStyle, x, y,
        width, height, hWnd, windowId, hInstance, nullptr));
    SendMessage(m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT); // Set extended style

    LV_COLUMN lvColumn{};
    lvColumn.mask = LVCF_WIDTH | LVCF_TEXT;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.cx = width;
    lvColumn.pszText = const_cast<wchar_t*>(L"Subscribed");
    SendMessage(m_hWnd, LVM_INSERTCOLUMN, 0, (LPARAM)&lvColumn);

    // Initialize a 'All feeds' item so that it can display all feeds from database
    LVITEM lvItem{};
    std::wstring text;
    lvItem.pszText = const_cast<wchar_t*>(L"All feeds");
    lvItem.iItem = 0;
    lvItem.iSubItem = 0;
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam = (LPARAM)FeedDatabase::INVALID_FEEDDATA_ID;
    SendMessage(m_hWnd, LVM_INSERTITEM, 0, (LPARAM)&lvItem);
}

void FeedListView::InsertFeed(const FeedDatabase::Feed& feed)
{
    LVITEM lvItem{};
    int col = 0;
    std::wstring text;
    FeedCommon::ConvertStringToWideString(feed.title, text);
    lvItem.pszText = text.data();
    lvItem.iItem = static_cast<int>(SendMessage(m_hWnd, LVM_GETITEMCOUNT, 0, 0));
    lvItem.iSubItem = col++;
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam = (LPARAM)feed.feedid;
    SendMessage(m_hWnd, LVM_INSERTITEM, 0, (LPARAM)&lvItem);
}

bool FeedListView::FeedIdExistInSet(long long feedid)
{
    ATL::CComCritSecLock lock(m_lock);
    return m_feedIdSet.end() != m_feedIdSet.find(feedid);
}

void FeedListView::InsertFeedIdIntoSet(long long feedid)
{
    ATL::CComCritSecLock lock(m_lock);
    m_feedIdSet.insert(feedid);
}

void FeedListView::ClearFeedIdSet()
{
    ATL::CComCritSecLock lock(m_lock);
    m_feedIdSet.clear();
}

LRESULT FeedListView::OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return m_fnProcessMessage(hWnd, message, wParam, lParam);
}
