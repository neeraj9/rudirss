#include "FeedItemListView.h"
#include "FeedCommon.h"
#include "FeedListView.h"
#include "RudiRSSMainWindow.h"
#include "resource.h"

#include <CommCtrl.h>
#include <format>

FeedItemListView::FeedItemListView() : m_titleColumnWidth{ 0 }, m_updatedColumnWidth{ 0 }, m_mainWindow{ nullptr }, m_lastRighClickedItem{ -1 }
{
}

FeedItemListView::FeedItemListView(RudiRSSMainWindow* mainWindow): m_titleColumnWidth{ 0 }, m_updatedColumnWidth{ 0 },
m_mainWindow{ mainWindow }, m_lastRighClickedItem{ -1 }
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
            if (0 == lpdi->item.iSubItem)
            {
                auto it = m_cache.find(lpdi->item.iItem);
                if (it != m_cache.end())
                {
                    std::wstring title;
                    FeedCommon::ConvertStringToWideString(it->second.title, title);
                    _snwprintf_s(lpdi->item.pszText, lpdi->item.cchTextMax, _TRUNCATE, L"%s", title.c_str());
                }
            }
            else if (1 == lpdi->item.iSubItem)
            {
                auto it = m_cache.find(lpdi->item.iItem);
                if (it != m_cache.end())
                {
                    std::wstring datetime;
                    FeedCommon::ConvertStringToWideString(it->second.datetime, datetime);
                    _snwprintf_s(lpdi->item.pszText, lpdi->item.cchTextMax, _TRUNCATE, L"%s", datetime.c_str());
                }
            }
        }
    }
    break;

    case LVN_ODCACHEHINT:
    {
        LPNMLVCACHEHINT pCachehint = (NMLVCACHEHINT*)lParam;
        if (FeedListView::ALL_FEEDS_LIST_INDEX == m_mainWindow->GetFeedListView().GetLastSelectedFeedIndex())
        {
            if (m_cache.empty())
            {
                if (m_mainWindow->GetLastSearchText().empty())
                {
                    long long idx = pCachehint->iFrom;
                    m_mainWindow->GetRudiRSSClient().QueryFeedDataOrderByTimestampInRange(static_cast<long long>(pCachehint->iTo - pCachehint->iFrom + 1),
                        static_cast<long long>(pCachehint->iFrom), [&](const FeedDatabase::FeedData& feedData) {
                            m_cache.insert(std::pair<long long, FeedDatabase::FeedData>(idx++, feedData));
                        });
                }
                else if(m_mainWindow->GetLastSearchResultCount() > 0)
                {
                    std::string query = std::format("%{}%", m_mainWindow->GetLastSearchText());
                    long long idx = pCachehint->iFrom;
                    m_mainWindow->GetRudiRSSClient().QueryFeedDataByTitleOrderByTimestampInRange(query,
                        static_cast<long long>(pCachehint->iTo - pCachehint->iFrom + 1),
                        static_cast<long long>(pCachehint->iFrom),
                        [&](const FeedDatabase::FeedData& feedData) {
                            m_cache.insert(std::pair<long long, FeedDatabase::FeedData>(idx++, feedData));
                        });
                }
            }
            else
            {
                auto insertionDirection = m_cache.GetInsertionDirection(static_cast<long long>(pCachehint->iFrom), static_cast<long long>(pCachehint->iTo));
                if (ListViewCache<FeedDatabase::FeedData>::InsertionDirection::NONE != insertionDirection)
                {
                    size_t cnt = static_cast<size_t>(pCachehint->iTo - pCachehint->iFrom + 1);
                    if (ListViewCache<FeedDatabase::FeedData>::InsertionDirection::FRONT == insertionDirection)
                        m_cache.DeleteBackElements(cnt);
                    else
                        m_cache.DeleteFrontElements(cnt);

                    if (m_mainWindow->GetLastSearchText().empty())
                    {
                        long long idx = pCachehint->iFrom;
                        m_mainWindow->GetRudiRSSClient().QueryFeedDataOrderByTimestampInRange(static_cast<long long>(pCachehint->iTo - pCachehint->iFrom + 1),
                            static_cast<long long>(pCachehint->iFrom), [&](const FeedDatabase::FeedData& feedData) {
                                m_cache.insert(std::pair<long long, FeedDatabase::FeedData>(idx++, feedData));
                            });
                    }
                    else if (m_mainWindow->GetLastSearchResultCount() > 0)
                    {
                        std::string query = std::format("%{}%", m_mainWindow->GetLastSearchText());
                        long long idx = pCachehint->iFrom;
                        m_mainWindow->GetRudiRSSClient().QueryFeedDataByTitleOrderByTimestampInRange(query,
                            static_cast<long long>(pCachehint->iTo - pCachehint->iFrom + 1),
                            static_cast<long long>(pCachehint->iFrom),
                            [&](const FeedDatabase::FeedData& feedData) {
                                m_cache.insert(std::pair<long long, FeedDatabase::FeedData>(idx++, feedData));
                            });
                    }
                }
            }
        }
        else
        {
            if (m_cache.empty())
            {
                if (m_mainWindow->GetLastSearchText().empty())
                {
                    long long idx = pCachehint->iFrom;
                    long long lastSelectedFeedId = m_mainWindow->GetFeedListView().GetLastSelectedFeedId();
                    m_mainWindow->GetRudiRSSClient().QueryFeedDataByFeedIdOrderByTimestampInRange(lastSelectedFeedId,
                        static_cast<long long>(pCachehint->iTo - pCachehint->iFrom + 1),
                        static_cast<long long>(pCachehint->iFrom), [&](const FeedDatabase::FeedData& feedData) {
                            m_cache.insert(std::pair<long long, FeedDatabase::FeedData>(idx++, feedData));
                        });
                }
                else if (m_mainWindow->GetLastSearchResultCount() > 0)
                {
                    long long idx = pCachehint->iFrom;
                    long long lastSelectedFeedId = m_mainWindow->GetFeedListView().GetLastSelectedFeedId();
                    std::string query = std::format("%{}%", m_mainWindow->GetLastSearchText());
                    m_mainWindow->GetRudiRSSClient().QueryFeedDataByFeedIdByTitleOrderByTimestampInRange(lastSelectedFeedId, query,
                        static_cast<long long>(pCachehint->iTo - pCachehint->iFrom + 1),
                        static_cast<long long>(pCachehint->iFrom), [&](const FeedDatabase::FeedData& feedData) {
                            m_cache.insert(std::pair<long long, FeedDatabase::FeedData>(idx++, feedData));
                        });
                }
            }
            else
            {
                auto insertionDirection = m_cache.GetInsertionDirection(static_cast<long long>(pCachehint->iFrom), static_cast<long long>(pCachehint->iTo));
                if (ListViewCache<FeedDatabase::FeedData>::InsertionDirection::NONE != insertionDirection)
                {
                    size_t cnt = static_cast<size_t>(pCachehint->iTo - pCachehint->iFrom + 1);
                    if (ListViewCache<FeedDatabase::FeedData>::InsertionDirection::FRONT == insertionDirection)
                        m_cache.DeleteBackElements(cnt);
                    else
                        m_cache.DeleteFrontElements(cnt);

                    if (m_mainWindow->GetLastSearchText().empty())
                    {
                        long long idx = pCachehint->iFrom;
                        long long lastSelectedFeedId = m_mainWindow->GetFeedListView().GetLastSelectedFeedId();
                        m_mainWindow->GetRudiRSSClient().QueryFeedDataByFeedIdOrderByTimestampInRange(lastSelectedFeedId,
                            static_cast<long long>(pCachehint->iTo - pCachehint->iFrom + 1),
                            static_cast<long long>(pCachehint->iFrom), [&](const FeedDatabase::FeedData& feedData) {
                                m_cache.insert(std::pair<long long, FeedDatabase::FeedData>(idx++, feedData));
                            });
                    }
                    else if (m_mainWindow->GetLastSearchResultCount() > 0)
                    {
                        long long idx = pCachehint->iFrom;
                        long long lastSelectedFeedId = m_mainWindow->GetFeedListView().GetLastSelectedFeedId();
                        std::string query = std::format("%{}%", m_mainWindow->GetLastSearchText());
                        m_mainWindow->GetRudiRSSClient().QueryFeedDataByFeedIdByTitleOrderByTimestampInRange(lastSelectedFeedId, query,
                            static_cast<long long>(pCachehint->iTo - pCachehint->iFrom + 1),
                            static_cast<long long>(pCachehint->iFrom), [&](const FeedDatabase::FeedData& feedData) {
                                m_cache.insert(std::pair<long long, FeedDatabase::FeedData>(idx++, feedData));
                            });
                    }
                }
            }
        }
    }
    break;

    case NM_CUSTOMDRAW:
    {
        LPNMLVCUSTOMDRAW lpCD = (LPNMLVCUSTOMDRAW)lParam;
        if (CDDS_PREPAINT == lpCD->nmcd.dwDrawStage)
        {
            return CDRF_NOTIFYITEMDRAW;
        }
        else if (CDDS_ITEMPREPAINT == lpCD->nmcd.dwDrawStage)
        {
            return CDRF_NOTIFYSUBITEMDRAW;
        }
        // Flag combined with CDDS_ITEMPREPAINT or CDDS_ITEMPOSTPAINT if a subitem is being drawn.
        else if ((CDDS_ITEMPREPAINT | CDDS_SUBITEM) == lpCD->nmcd.dwDrawStage)
        {
            if (0 == lpCD->iSubItem)
            {
                auto it = m_cache.find(lpCD->nmcd.dwItemSpec);
                if (it != m_cache.end())
                    SelectObject(lpCD->nmcd.hdc, 1 == it->second.read ? m_mainWindow->GetDefaultFont(): m_mainWindow->GetBoldFont());
            }

            return CDRF_NEWFONT;
        }

        return CDRF_SKIPDEFAULT;
    }
    break;

    case NM_CLICK:
    {
        LPNMITEMACTIVATE itemActivate = (LPNMITEMACTIVATE)lParam;
        if (m_mainWindow->IsViewerInitialized())
        {
            auto it = m_cache.find(itemActivate->iItem);
            if (it != m_cache.end())
            {
                it->second.read = static_cast<long long>(true);
                m_mainWindow->GetRudiRSSClient().UpdateFeedDataReadColumn(it->second.feeddataid, static_cast<long long>(true));
                ListView_Update(m_hWnd, itemActivate->iItem);

                std::wstring link;
                FeedCommon::ConvertStringToWideString(it->second.link, link);

                // To handle Protocol-Relative link
                if (L"//" == link.substr(0, 2))
                    link.insert(0, L"https:");
                m_mainWindow->GetViewer().Navigate(link);
            }
        }
    }
    break;

    case NM_RCLICK:
    {
        LPNMITEMACTIVATE itemActivate = (LPNMITEMACTIVATE)lParam;
        if (-1 != itemActivate->iItem)
        {
            m_lastRighClickedItem = itemActivate->iItem;
            HMENU hPopupMenu = LoadMenu(m_mainWindow->GetHInstance(), MAKEINTRESOURCE(IDR_FEED_MENU));
            if (hPopupMenu)
            {
                POINT pt{};
                if (GetCursorPos(&pt))
                {
                    HMENU hMenu = GetSubMenu(hPopupMenu, 0);
                    TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
                }
                DestroyMenu(hPopupMenu);
            }
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
    DeleteAllItems();
    long long cnt = 0;
    if (m_mainWindow->GetLastSearchText().empty())
    {
        m_mainWindow->GetRudiRSSClient().QueryFeedDataTableCountByFeedId(feedid, cnt);
    }
    else if (m_mainWindow->GetLastSearchResultCount() > 0)
    {
        std::string query = std::format("%{}%", m_mainWindow->GetLastSearchText());
        m_mainWindow->GetRudiRSSClient().QueryFeedDataCountByFeedIdByTitle(feedid, query, cnt);
    }

    if (cnt > 0)
    {
        ListView_SetItemCount(m_hWnd, cnt);
    }
}

void FeedItemListView::UpdateAllFeeds()
{
    DeleteAllItems();
    long long cnt = 0;
    if (m_mainWindow->GetLastSearchText().empty())
    {
        m_mainWindow->GetRudiRSSClient().QueryFeedDataTableCount(cnt);
    }
    else if (m_mainWindow->GetLastSearchResultCount() > 0)
    {
        std::string query = std::format("%{}%", m_mainWindow->GetLastSearchText());
        m_mainWindow->GetRudiRSSClient().QueryFeedDataCountByTitle(query, cnt);
    }

    if (cnt > 0)
    {
        ListView_SetItemCount(m_hWnd, cnt);
    }
}

void FeedItemListView::DeleteAllItems()
{
    ClearCache();
    SendMessage(m_hWnd, LVM_DELETEALLITEMS, 0, 0);
}

void FeedItemListView::ClearCache()
{
    m_cache.clear();
}

void FeedItemListView::UpdateReadStateInCache(int item, long long read)
{
    auto it = m_cache.find(item);
    if (it != m_cache.end())
    {
        it->second.read = read;
    }
    ListView_Update(m_hWnd, item);
}
