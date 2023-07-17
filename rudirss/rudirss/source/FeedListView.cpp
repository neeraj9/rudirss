#include "FeedListView.h"
#include "FeedCommon.h"
#include "Resource.h"

#include <atlbase.h>
#include <CommCtrl.h>

FeedListView::FeedListView()
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

    constexpr DWORD dwStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_NOSORTHEADER;
    Attach(CreateWindow(WC_LISTVIEW, nullptr, dwStyle, x, y,
        width, height, hWnd, windowId, hInstance, nullptr));
    SendMessage(m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER); // Set extended style

    LV_COLUMN lvColumn{};
    lvColumn.mask = LVCF_WIDTH | LVCF_TEXT;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.cx = width;
    lvColumn.pszText = const_cast<wchar_t*>(L"Subscribed");
    SendMessage(m_hWnd, LVM_INSERTCOLUMN, 0, (LPARAM)&lvColumn);
}

LRESULT FeedListView::OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return m_fnProcessMessage(hWnd, message, wParam, lParam);
}
