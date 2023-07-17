#include "FeedItemListView.h"
#include "FeedCommon.h"
#include "FeedListView.h"
#include "RudiRSSMainWindow.h"

#include <CommCtrl.h>

FeedItemListView::FeedItemListView() : m_titleColumnWidth{ 0 }, m_updatedColumnWidth{ 0 }, m_mainWindow{ nullptr }
{
}

FeedItemListView::FeedItemListView(RudiRSSMainWindow* mainWindow): m_titleColumnWidth{ 0 }, m_updatedColumnWidth{ 0 },
m_mainWindow{ mainWindow }
{

}

FeedItemListView::~FeedItemListView()
{

}

void FeedItemListView::Initialize(HWND hWnd, HINSTANCE hInstance, HMENU windowId, int x, int y, int width, int height,
    int titleColWidth, int updatedColWidth)
{
    m_width = width;
    m_height = height;
    m_titleColumnWidth = titleColWidth;
    m_updatedColumnWidth = updatedColWidth;

    constexpr DWORD dwStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_NOSORTHEADER;
    Attach(CreateWindow(WC_LISTVIEW, nullptr, dwStyle, x, y,
        width, height, hWnd, windowId, hInstance, nullptr));
    SendMessage(m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER); // Set extended style

    LV_COLUMN lvColumn{};
    lvColumn.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
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
    LPNMHDR lpNMHDR = (LPNMHDR)lParam;
    switch (lpNMHDR->code)
    {
    case LVN_GETDISPINFO:
    {
        LV_DISPINFO* lpdi = (LV_DISPINFO*)lParam;
        if (lpdi->item.mask & LVIF_TEXT)
        {
            if (FeedListView::ALL_FEEDS_LIST_INDEX == m_mainWindow->GetFeedListView().GetLastSelectedFeedIndex())
            {
                if (0 == lpdi->item.iSubItem)
                {
                    m_mainWindow->GetRudiRSSClient().QueryFeedDataByOffsetOrderByTimestamp(lpdi->item.iItem,
                        [&](const FeedDatabase::FeedData& feedData) {
                            std::wstring title;
                            FeedCommon::ConvertStringToWideString(feedData.title, title);
                            title.insert(0, feedData.read ? L"[X] " : L"[ ] ");
                            _snwprintf_s(lpdi->item.pszText, lpdi->item.cchTextMax, _TRUNCATE, L"%s", title.c_str());
                        });
                }
                else if (1 == lpdi->item.iSubItem)
                {
                    m_mainWindow->GetRudiRSSClient().QueryFeedDataByOffsetOrderByTimestamp(lpdi->item.iItem,
                        [&](const FeedDatabase::FeedData& feedData) {
                            std::wstring datetime;
                            FeedCommon::ConvertStringToWideString(feedData.datetime, datetime);
                            _snwprintf_s(lpdi->item.pszText, lpdi->item.cchTextMax, _TRUNCATE, L"%s", datetime.c_str());
                        });
                }
            }
            else
            {
                long long lastSelectedFeedId = m_mainWindow->GetFeedListView().GetLastSelectedFeedId();
                if (0 == lpdi->item.iSubItem)
                {
                    m_mainWindow->GetRudiRSSClient().QueryFeedDataByFeedIdByOffsetOrderByTimestamp(lastSelectedFeedId, lpdi->item.iItem,
                        [&](const FeedDatabase::FeedData& feedData) {
                            std::wstring title;
                            FeedCommon::ConvertStringToWideString(feedData.title, title);
                            title.insert(0, feedData.read ? L"[X] " : L"[ ] ");
                            _snwprintf_s(lpdi->item.pszText, lpdi->item.cchTextMax, _TRUNCATE, L"%s", title.c_str());
                        });
                }
                else if (1 == lpdi->item.iSubItem)
                {
                    m_mainWindow->GetRudiRSSClient().QueryFeedDataByFeedIdByOffsetOrderByTimestamp(lastSelectedFeedId, lpdi->item.iItem,
                        [&](const FeedDatabase::FeedData& feedData) {
                            std::wstring datetime;
                            FeedCommon::ConvertStringToWideString(feedData.datetime, datetime);
                            _snwprintf_s(lpdi->item.pszText, lpdi->item.cchTextMax, _TRUNCATE, L"%s", datetime.c_str());
                        });
                }
            }
        }
    }
    break;

    case NM_CLICK:
    {
        LPNMITEMACTIVATE itemActivate = (LPNMITEMACTIVATE)lParam;
        if (m_mainWindow->IsViewerInitialized())
        {
            std::wstring title;
            std::wstring link;
            long long feeddataid = FeedDatabase::INVALID_FEEDDATA_ID;
            if (FeedListView::ALL_FEEDS_LIST_INDEX == m_mainWindow->GetFeedListView().GetLastSelectedFeedIndex())
            {
                m_mainWindow->GetRudiRSSClient().QueryFeedDataByOffsetOrderByTimestamp(itemActivate->iItem,
                    [&](const FeedDatabase::FeedData& feedData) {
                        feeddataid = feedData.feeddataid;
                        FeedCommon::ConvertStringToWideString(feedData.title, title);
                        FeedCommon::ConvertStringToWideString(feedData.link, link);
                    });
            }
            else
            {
                m_mainWindow->GetRudiRSSClient().QueryFeedDataByFeedIdByOffsetOrderByTimestamp(m_mainWindow->GetFeedListView().GetLastSelectedFeedId(),
                    itemActivate->iItem,
                    [&](const FeedDatabase::FeedData& feedData) {
                        feeddataid = feedData.feeddataid;
                        FeedCommon::ConvertStringToWideString(feedData.title, title);
                        FeedCommon::ConvertStringToWideString(feedData.link, link);
                    });
            }
            m_mainWindow->GetRudiRSSClient().UpdateFeedDataReadColumn(feeddataid, static_cast<long long>(true));
            // To handle Protocol-Relative link
            if (L"//" == link.substr(0, 2))
                link.insert(0, L"https:");
            m_mainWindow->GetViewer().Navigate(link);
        }
    }
    break;

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void FeedItemListView::UpdateSelectedFeed(long long feedid)
{
    SendMessage(m_hWnd, LVM_DELETEALLITEMS, 0, 0);
    long long cnt = 0;
    m_mainWindow->GetRudiRSSClient().QueryFeedDataTableCountByFeedId(feedid, cnt);
    if (cnt > 0)
    {
        ListView_SetItemCount(m_hWnd, cnt);
    }
}

void FeedItemListView::UpdateAllFeeds()
{
    SendMessage(m_hWnd, LVM_DELETEALLITEMS, 0, 0);
    long long cnt = 0;
    m_mainWindow->GetRudiRSSClient().QueryFeedDataTableCount(cnt);
    if (cnt > 0)
    {
        ListView_SetItemCount(m_hWnd, cnt);
    }
}

