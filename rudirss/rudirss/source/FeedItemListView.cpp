#include "FeedItemListView.h"
#include "FeedCommon.h"

FeedItemListView::FeedItemListView() : m_titleColumnWidth{ 0 }, m_updatedColumnWidth{ 0 }
{
}

FeedItemListView::~FeedItemListView()
{

}

void FeedItemListView::Initialize(HWND hWnd, HINSTANCE hInstance, HMENU windowId, int x, int y, int width, int height,
    int titleColWidth, int updatedColWidth, FN_PROCESS_MESSAGE fnProcessMessage)
{
    m_width = width;
    m_height = height;
    m_titleColumnWidth = titleColWidth;
    m_updatedColumnWidth = updatedColWidth;
    m_fnProcessMessage = fnProcessMessage;

    constexpr DWORD dwStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS;
    Attach(CreateWindowEx(WS_EX_STATICEDGE, WC_LISTVIEW, nullptr, dwStyle, x, y,
        width, height, hWnd, windowId, hInstance, nullptr));
    SendMessage(m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT); // Set extended style

    LV_COLUMN lvColumn{};
    lvColumn.mask = LVCF_WIDTH | LVCF_TEXT;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.cx = titleColWidth;
    lvColumn.pszText = const_cast<wchar_t*>(L"Title");
    SendMessage(m_hWnd, LVM_INSERTCOLUMN, 0, (LPARAM)&lvColumn);

    lvColumn.cx = updatedColWidth;
    lvColumn.pszText = const_cast<wchar_t*>(L"Updated");
    SendMessage(m_hWnd, LVM_INSERTCOLUMN, 1, (LPARAM)&lvColumn);
}

void FeedItemListView::InsertFeedItem(const FeedDatabase::FeedData& feedData)
{
    LVITEM lvItem{};
    int col = 0;
    std::wstring text;
    FeedCommon::ConvertStringToWideString(feedData.title, text);
    text = GetReadStateSymbol(feedData.read) + text;
    lvItem.pszText = text.data();
    lvItem.iItem = static_cast<int>(SendMessage(m_hWnd, LVM_GETITEMCOUNT, 0, 0));
    lvItem.iSubItem = col++;
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam = (LPARAM)feedData.feeddataid;
    SendMessage(m_hWnd, LVM_INSERTITEM, 0, (LPARAM)&lvItem);

    FeedCommon::ConvertStringToWideString(feedData.datetime, text);
    lvItem.pszText = text.data();
    lvItem.iSubItem = col;
    lvItem.mask = LVIF_TEXT;
    SendMessage(m_hWnd, LVM_SETITEM, 0, (LPARAM)&lvItem);
}

std::wstring FeedItemListView::GetReadStateSymbol(long long read)
{
    return 0 != read ? L"[X] " : L"[ ] ";
}

void FeedItemListView::MarkFeedDataAsReadOrUnRead(LPNMITEMACTIVATE activateItem, const std::wstring& title, long long read)
{
    std::wstring text = GetReadStateSymbol(read) + title;;
    LVITEM lvItem{};
    lvItem.iItem = activateItem->iItem;
    lvItem.iSubItem = activateItem->iSubItem;
    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = text.data();
    SendMessage(activateItem->hdr.hwndFrom, LVM_SETITEM, 0, (LPARAM)&lvItem);
}

LRESULT FeedItemListView::OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return m_fnProcessMessage(hWnd, message, wParam, lParam);
}
