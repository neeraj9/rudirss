#include "RudiRSSMainWindow.h"
#include "Resource.h"
#include "FeedBase.h"
#include "ListView.h"

RudiRSSMainWindow::RudiRSSMainWindow() : m_initViewer{ FALSE }, m_font{ nullptr }
{
}

RudiRSSMainWindow::~RudiRSSMainWindow()
{
}

void RudiRSSMainWindow::OnRegister(WNDCLASSEXW& wcex)
{
    WCHAR title[MAX_PATH]{};
    WCHAR windowClass[MAX_PATH]{};
    LoadStringW(m_hInstance, IDS_APP_TITLE, title, _countof(title));
    LoadStringW(m_hInstance, IDC_RUDIRSS, windowClass, _countof(windowClass));
    m_title = title;
    m_className = windowClass;

    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MainWindow::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = m_hInstance;
    wcex.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_RUDIRSS));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    //wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_RUDIRSS);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = m_className.c_str();
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_RUDIRSS));
}

HWND RudiRSSMainWindow::Create()
{
    HWND hWnd = CreateWindowW(m_className.c_str(), m_title.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, m_hInstance, nullptr);

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    return hWnd;
}

bool RudiRSSMainWindow::Initialize(HINSTANCE hInstance)
{
    bool result = false;
    if (MainWindow::Initialize(hInstance))
    {
        InittializeControl();

        result = m_rudiRSSClient.Initialize();

        if (result)
        {
            m_feedListView.ClearFeedIdSet();
            m_rudiRSSClient.QueryAllFeeds([&](const FeedDatabase::Feed& feed) {
                m_feedListView.InsertFeedIdIntoSet(feed.feedid);
                m_feedListView.InsertFeed(feed);
                });

            m_rudiRSSClient.StartRefreshFeedTimer([&](const FeedDatabase::FeedConsumptionUnit& consumptionUnit) {
                if (FeedDatabase::FeedConsumptionUnit::OperationType::NOTIFY_INSERTION_COMPLETE == consumptionUnit.opType)
                {
                    if (!m_feedListView.FeedIdExistInSet(consumptionUnit.feed.feedid))
                    {
                        m_feedListView.InsertFeed(consumptionUnit.feed);
                    }
                    else
                    {
                        // Update the feed items in FeedItemListView that belong to the selected feed in FeedListView
                        auto selectedIndex = SendMessage(m_feedListView.m_hWnd, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
                        if (-1 != selectedIndex)
                        {
                            auto feedid = static_cast<long long>(ListView::GetLParamFromSelectedItem(m_feedListView.m_hWnd, selectedIndex));
                            if (consumptionUnit.feed.feedid == feedid)
                            {
                                // Only update the selected feed to which feedid belongs
                                UpdateSelectedFeed(feedid);
                            }
                        }
                    }
                }
                });
        }
    }

    return result;
}

LRESULT RudiRSSMainWindow::OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_NOTIFY:
    {
        return OnProcessListViewCommand(hWnd, message, wParam, lParam);
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_SIZE:
        UpdateControl();
        break;

    case WM_DESTROY:
        OnDestroy();
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

void RudiRSSMainWindow::OnDestroy()
{
    DeleteObject(m_font);
}

void RudiRSSMainWindow::InittializeControl()
{
    INITCOMMONCONTROLSEX icex{};
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    InitFont();

    RECT rc{};
    GetClientRect(m_hWnd, &rc);
    LONG height = rc.bottom - rc.top;
    LONG viewerX = 0;

    m_feedListView.Initialize(m_hWnd, m_hInstance, (HMENU)IDC_FEED_LIST_VIEW, 0, 0, 300, height,
        [&](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT {
            LPNMITEMACTIVATE itemActivate = (LPNMITEMACTIVATE)lParam;
            switch (itemActivate->hdr.code)
            {
            case NM_CLICK:
            {
                long long feedid = static_cast<long long>(ListView::GetLParamFromActivatedItem(itemActivate));
                UpdateSelectedFeed(feedid);
            }
            break;

            default:
                break;
            }

            return DefWindowProc(hWnd, message, wParam, lParam);
        });
    SendMessage(m_feedListView.m_hWnd, WM_SETFONT, (WPARAM)m_font, TRUE);
    GetClientRect(m_feedListView.m_hWnd, &rc);
    viewerX += rc.right - rc.left;

    const int titleColWidth = 250;
    const int updatedColWidth = 150;
    m_feedItemListView.Initialize(m_hWnd, m_hInstance, (HMENU)IDC_FEED_ITEM_LIST_VIEW, rc.right + 1, 0,
        titleColWidth + updatedColWidth, height, titleColWidth, updatedColWidth,
        [&](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT {
            LPNMITEMACTIVATE itemActivate = (LPNMITEMACTIVATE)lParam;
            switch (itemActivate->hdr.code)
            {
            case NM_CLICK:
            {
                if (InterlockedOr(reinterpret_cast<LONG*>(&m_initViewer), 0))
                {
                    long long feedDataId = static_cast<long long>(ListView::GetLParamFromActivatedItem(itemActivate));
                    std::wstring title;
                    m_rudiRSSClient.QueryFeedDataByFeedDataId(feedDataId, [&](const FeedDatabase::FeedData& feedData) {
                        FeedCommon::ConvertStringToWideString(feedData.title, title);
                        std::wstring link;
                        FeedCommon::ConvertStringToWideString(feedData.link, link);
                        m_viewer.Navigate(link);
                        });
                    if (m_rudiRSSClient.UpdateFeedDataReadColumn(feedDataId, static_cast<long long>(true)))
                        m_feedItemListView.MarkFeedDataAsReadOrUnRead(itemActivate, title, static_cast<long long>(true));
                }
            }
            break;

            default:
                break;
            }

            return DefWindowProc(hWnd, message, wParam, lParam);
        });
    SendMessage(m_feedItemListView.m_hWnd, WM_SETFONT, (WPARAM)m_font, TRUE);
    GetClientRect(m_feedItemListView.m_hWnd, &rc);

    viewerX += rc.right - rc.left;
    RECT viewerRect{};
    GetClientRect(m_hWnd, &viewerRect);
    viewerRect.left = viewerX + 1;
    m_viewer.Initialize(m_hWnd, viewerRect, [&]() {
        InterlockedExchange(reinterpret_cast<LONG*>(&m_initViewer), TRUE);
        });
}

void RudiRSSMainWindow::InitFont()
{
    LOGFONT font{};
    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(font), &font, 0);
    m_font = CreateFont(font.lfHeight, font.lfWidth, font.lfEscapement, font.lfOrientation, font.lfWeight, font.lfItalic, font.lfUnderline, font.lfStrikeOut,
        font.lfCharSet, font.lfOutPrecision, font.lfClipPrecision, font.lfQuality, font.lfPitchAndFamily, font.lfFaceName);
}

void RudiRSSMainWindow::UpdateSelectedFeed(long long feedid)
{
    SendMessage(m_feedItemListView.m_hWnd, LVM_DELETEALLITEMS, 0, 0);
    if (FeedDatabase::INVALID_FEEDDATA_ID != feedid)
    {
        m_rudiRSSClient.QueryFeedDataOrderByTimestamp(feedid, [&](const FeedDatabase::FeedData& feedData) {
            m_feedItemListView.InsertFeedItem(feedData);
            });
    }
    else
    {
        m_rudiRSSClient.QueryAllFeedDataOrderByTimestamp([&](const FeedDatabase::FeedData& feedData) {
            m_feedItemListView.InsertFeedItem(feedData);
            });
    }
}

void RudiRSSMainWindow::UpdateControl()
{
    if (!m_hWnd)
        return;

    do
    {
        RECT rc{};
        GetClientRect(m_hWnd, &rc);
        LONG height = rc.bottom - rc.top;
        LONG viewerX = 0;

        if (!m_feedListView.m_hWnd)
            break;

        MoveWindow(m_feedListView.m_hWnd, 0, 0, m_feedListView.GetWidth(), height, FALSE);
        ::RedrawWindow(m_feedListView.m_hWnd, nullptr, nullptr, RDW_INVALIDATE);
        GetClientRect(m_feedListView.m_hWnd, &rc);
        viewerX += rc.right - rc.left;

        if (!m_feedItemListView.m_hWnd)
            break;

        MoveWindow(m_feedItemListView.m_hWnd, rc.right + 1, 0, m_feedItemListView.GetWidth(), height, FALSE);
        ::RedrawWindow(m_feedItemListView.m_hWnd, nullptr, nullptr, RDW_INVALIDATE);
        GetClientRect(m_feedItemListView.m_hWnd, &rc);
        viewerX += rc.right - rc.left;

        if (InterlockedOr(reinterpret_cast<LONG*>(&m_initViewer), 0))
        {
            RECT viewerRect{};
            GetClientRect(m_hWnd, &viewerRect);
            viewerRect.left = viewerX + 1;
            m_viewer.MoveWindow(viewerRect);
        }
    } while (0);
}

LRESULT RudiRSSMainWindow::OnProcessListViewCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
    case IDC_FEED_LIST_VIEW:
    {
        return m_feedListView.OnProcessMessage(hWnd, message, wParam, lParam);
    }
    break;

    case IDC_FEED_TITLE_LIST_VIEW:
    {
        return m_feedItemListView.OnProcessMessage(hWnd, message, wParam, lParam);
    }

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
