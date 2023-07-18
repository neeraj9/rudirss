#include "FeedListView.h"
#include "FeedCommon.h"
#include "FeedDatabase.h"
#include "RudiRSSMainWindow.h"
#include "Resource.h"

#include <atlbase.h>
#include <CommCtrl.h>

FeedListView::FeedListView() : m_mainWindow{ nullptr }, m_lastSelectedFeedId{ FeedDatabase::INVALID_FEED_ID }, m_lastSelectedFeedIndex{ -1 }
{
}

FeedListView::FeedListView(RudiRSSMainWindow* mainWindow) : m_mainWindow{ mainWindow }, 
m_lastSelectedFeedId{ FeedDatabase::INVALID_FEED_ID }, m_lastSelectedFeedIndex{ -1 }
{

}

FeedListView::~FeedListView()
{

}

void FeedListView::Initialize(HWND hWnd, HINSTANCE hInstance, HMENU windowId, int x, int y, int width, int height)
{
    m_width = width;
    m_height = height;

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
    LPNMITEMACTIVATE itemActivate = (LPNMITEMACTIVATE)lParam;
    switch (itemActivate->hdr.code)
    {
    case LVN_GETDISPINFO:
    {
        LV_DISPINFO* lpdi = (LV_DISPINFO*)lParam;
        if (0 == lpdi->item.iSubItem
            && lpdi->item.mask & LVIF_TEXT)
        {
            if (ALL_FEEDS_LIST_INDEX == lpdi->item.iItem)
            {
                _snwprintf_s(lpdi->item.pszText, lpdi->item.cchTextMax, _TRUNCATE, L"All feeds");
            }
            else
            {
                auto it = m_cache.find(static_cast<long long>(lpdi->item.iItem - 1));
                if (it != m_cache.end())
                {
                    std::wstring title;
                    FeedCommon::ConvertStringToWideString(it->second.title, title);
                    _snwprintf_s(lpdi->item.pszText, lpdi->item.cchTextMax, _TRUNCATE, L"%s", title.c_str());
                }
            }
        }
    }
    break;


    case LVN_ODCACHEHINT:
    {
        LPNMLVCACHEHINT pCachehint = (NMLVCACHEHINT*)lParam;
        long long idx = pCachehint->iFrom;
        m_mainWindow->GetRudiRSSClient().QueryFeedByOffsetInRange(static_cast<long long>(pCachehint->iTo - pCachehint->iFrom + 1),
            static_cast<long long>(pCachehint->iFrom),
            [&](const FeedDatabase::Feed& feed) {
                m_cache.insert(std::pair<long long, FeedDatabase::Feed>(idx++, feed));
            });
    }
    break;

    case NM_CLICK:
    {
        InterlockedExchange(reinterpret_cast<long*>(&m_lastSelectedFeedIndex), itemActivate->iItem);
        if (ALL_FEEDS_LIST_INDEX != itemActivate->iItem)
        {
            m_mainWindow->GetRudiRSSClient().QueryFeedByOffset(itemActivate->iItem - 1, [&](const FeedDatabase::Feed& feed) {
                InterlockedExchange64(reinterpret_cast<LONG64*>(&m_lastSelectedFeedId), feed.feedid);
                });

            m_mainWindow->GetFeedItemListView().UpdateSelectedFeed(InterlockedOr64(reinterpret_cast<LONG64*>(&m_lastSelectedFeedId), 0));
        }
        else
        {
            m_mainWindow->GetFeedItemListView().UpdateAllFeeds();
        }
    }
    break;

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

long long FeedListView::GetLastSelectedFeedId()
{
    return InterlockedOr64(reinterpret_cast<LONG64*>(&m_lastSelectedFeedId), 0);
}

int FeedListView::GetLastSelectedFeedIndex()
{
    return InterlockedOr(reinterpret_cast<long*>(&m_lastSelectedFeedIndex), 0);
}

void FeedListView::UpdateFeedListFromDatabase()
{
    ClearCache();
    SendMessage(m_hWnd, LVM_DELETEALLITEMS, 0, 0);
    long long cnt = 0;
    m_mainWindow->GetRudiRSSClient().QueryFeedTableCount(cnt);
    if (cnt > 0)
    {
        ListView_SetItemCount(m_hWnd, cnt + 1); // Plus one for 'All feeds'
    }
}

void FeedListView::UpdateFeedList(const std::vector<std::wstring>& feedUrls)
{
    long long differentCount = 0;
    for (const auto& url : feedUrls)
    {
        std::string guid;
        FeedCommon::ConvertWideStringToString(url, guid);
        long long exist = 0;
        m_mainWindow->GetRudiRSSClient().QueryFeedExistByGuid(guid, exist);
        if (0 == exist)
        {
            differentCount++;
        }
    }

    long long feedCount = 0;
    if (m_mainWindow->GetRudiRSSClient().QueryFeedTableCount(feedCount))
    {
        ClearCache();
        ListView_SetItemCount(m_hWnd, feedCount + differentCount + 1); // Plus one for 'All feeds'
    }
}

void FeedListView::ClearCache()
{
    m_cache.clear();
}
