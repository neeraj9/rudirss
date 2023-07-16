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

    constexpr DWORD dwStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_NOSORTHEADER;
    Attach(CreateWindowEx(WS_EX_STATICEDGE, WC_LISTVIEW, nullptr, dwStyle, x, y,
        width, height, hWnd, windowId, hInstance, nullptr));
    SendMessage(m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER); // Set extended style

    LV_COLUMN lvColumn{};
    lvColumn.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    //lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVIF_PARAM;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.cx = titleColWidth;
    lvColumn.pszText = const_cast<wchar_t*>(L"Title");
    SendMessage(m_hWnd, LVM_INSERTCOLUMN, 0, (LPARAM)&lvColumn);

    lvColumn.cx = updatedColWidth;
    lvColumn.pszText = const_cast<wchar_t*>(L"Updated");
    SendMessage(m_hWnd, LVM_INSERTCOLUMN, 1, (LPARAM)&lvColumn);
}

LRESULT FeedItemListView::OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return m_fnProcessMessage(hWnd, message, wParam, lParam);
}
