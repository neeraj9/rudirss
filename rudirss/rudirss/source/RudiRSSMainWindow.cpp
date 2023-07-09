#include "RudiRSSMainWindow.h"
#include "Resource.h"
#include "FeedBase.h"

#include <format>
#include <vector>

RudiRSSMainWindow::RudiRSSMainWindow() : m_initViewer{ FALSE }, m_font{ nullptr }
{
    m_feedListLock.Init();
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
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
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
            ClearFeedIdSet();
            m_rudiRSSClient.QueryAllFeeds([&](const FeedDatabase::Feed& feed) {
                InsertFeedIdIntoSet(feed.feedid);

                std::wstring title;
                FeedCommon::ConvertStringToWideString(feed.title, title);
                int pos = (int)SendMessage(m_feedListBox.m_hWnd, LB_ADDSTRING, 0,
                    (LPARAM)title.c_str());
                SendMessage(m_feedListBox.m_hWnd, LB_SETITEMDATA, pos, (LPARAM)feed.feedid);
                });

            m_rudiRSSClient.StartRefreshFeedTimer(0, 1800 * 1000, [&](const FeedDatabase::FeedConsumptionUnit& consumptionUnit) {
                if (FeedDatabase::FeedConsumptionUnit::OperationType::NOTIFY_INSERTION_COMPLETE == consumptionUnit.opType)
                {
                    m_rudiRSSClient.QueryFeed(consumptionUnit.feed.guid, [&](const FeedDatabase::Feed& feed) {
                        if (!FeedIdExistInSet(feed.feedid))
                        {
                            std::wstring title;
                            FeedCommon::ConvertStringToWideString(feed.title, title);
                            int pos = (int)SendMessage(m_feedListBox.m_hWnd, LB_ADDSTRING, 0,
                                (LPARAM)title.c_str());
                            SendMessage(m_feedListBox.m_hWnd, LB_SETITEMDATA, pos, (LPARAM)feed.feedid);
                        }
                        });
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
    case WM_COMMAND:
    {
        return OnListBoxCommand(hWnd, message, wParam, lParam);
    }

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
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

void RudiRSSMainWindow::InittializeControl()
{
    InitFont();

    RECT rc{};
    GetClientRect(m_hWnd, &rc);
    LONG height = rc.bottom - rc.top;
    LONG viewerX = 0;

    InitFeedListBox(0, 0, 300, height);
    GetClientRect(m_feedListBox.m_hWnd, &rc);
    viewerX += rc.right - rc.left;

    int titleColWidth = 200;
    int updatedColWidth = 100;
    InitFeedTitleListView(rc.right + 1, 0, titleColWidth + updatedColWidth, height, titleColWidth, updatedColWidth);
    GetClientRect(m_feedTitleListView.m_hWnd, &rc);

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

void RudiRSSMainWindow::InitFeedListBox(int x, int y, int width, int height)
{
    m_feedListBox.Attach(CreateWindowExW(WS_EX_STATICEDGE,//WS_EX_CLIENTEDGE,
        L"LISTBOX", nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | LBS_NOTIFY,
        x, y, width, height, m_hWnd, (HMENU)IDC_FEED_LIST, m_hInstance, NULL));

    SendMessage(m_feedListBox.m_hWnd, WM_SETFONT, (WPARAM)m_font, TRUE);
}

void RudiRSSMainWindow::InitFeedTitleListView(int x, int y, int width, int height, int titleColWidth, int updatedColWidth)
{
    INITCOMMONCONTROLSEX icex{};
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    DWORD dwStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT;
    m_feedTitleListView.Attach(CreateWindowEx(WS_EX_STATICEDGE, WC_LISTVIEW, nullptr, dwStyle, x, y,
        titleColWidth + updatedColWidth, height, m_hWnd, (HMENU)IDC_FEED_TITLE_LIST_VIEW, m_hInstance, nullptr));
    SendMessage(m_feedTitleListView.m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT); // Set extended style

    LV_COLUMN   lvColumn{};
    lvColumn.mask = LVCF_WIDTH | LVCF_TEXT;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.cx = titleColWidth;
    lvColumn.pszText = const_cast<wchar_t*>(L"Title");
    SendMessage(m_feedTitleListView.m_hWnd, LVM_INSERTCOLUMN, 0, (LPARAM)&lvColumn);

    lvColumn.cx = updatedColWidth;
    lvColumn.pszText = const_cast<wchar_t*>(L"Updated");
    SendMessage(m_feedTitleListView.m_hWnd, LVM_INSERTCOLUMN, 1, (LPARAM)&lvColumn);
}

void RudiRSSMainWindow::InsertIntoFeedTitleListView(const FeedDatabase::FeedData& feedData)
{
    unsigned int listCnt = (unsigned int)SendMessage(m_feedTitleListView.m_hWnd, LVM_GETITEMCOUNT, 0, 0);
    if (listCnt > 0)
        listCnt--;

    LVITEM lvItem{};
    int col = 0;
    std::wstring text;
    FeedCommon::ConvertStringToWideString(feedData.title, text);
    lvItem.pszText = text.data();
    lvItem.iItem = listCnt;
    lvItem.iSubItem = col++;
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam = (LPARAM)feedData.feeddataid;
    SendMessage(m_feedTitleListView.m_hWnd, LVM_INSERTITEM, 0, (LPARAM)&lvItem);

    FeedCommon::ConvertStringToWideString(feedData.datetime, text);
    lvItem.pszText = text.data();
    lvItem.iSubItem = col;
    lvItem.mask = LVIF_TEXT;
    SendMessage(m_feedTitleListView.m_hWnd, LVM_SETITEM, 0, (LPARAM)&lvItem);
}

long long RudiRSSMainWindow::GetFeedIdFromFeedTitleListView()
{
    long long feeddataid = 0;
    int selItem = (int)SendMessage(m_feedTitleListView.m_hWnd, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
    if (-1 != selItem)
    {
        LVITEM lvItem{};
        lvItem.iItem = selItem;
        lvItem.iSubItem = 0;
        lvItem.mask = LVIF_PARAM;
        SendMessage(m_feedTitleListView.m_hWnd, LVM_GETITEM, 0, (LPARAM)&lvItem);
        feeddataid = (long long)lvItem.lParam;
    }

    return feeddataid;
}

LPARAM RudiRSSMainWindow::GetLParamFromListView(LPNMITEMACTIVATE activateItem)
{
    LVITEM lvItem{};
    lvItem.iItem = activateItem->iItem;
    lvItem.iSubItem = activateItem->iSubItem;
    lvItem.mask = LVIF_PARAM;
    SendMessage(activateItem->hdr.hwndFrom, LVM_GETITEM, 0, (LPARAM)&lvItem);
    return lvItem.lParam;
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
        if (!m_feedListBox.m_hWnd)
            break;

        MoveWindow(m_feedListBox.m_hWnd, 0, 0, 300, height, FALSE);
        GetClientRect(m_feedListBox.m_hWnd, &rc);
        viewerX += rc.right - rc.left;

        if (!m_feedTitleListView.m_hWnd)
            break;

        MoveWindow(m_feedTitleListView.m_hWnd, rc.right + 1, 0, 300, height, FALSE);
        GetClientRect(m_feedTitleListView.m_hWnd, &rc);
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

LRESULT RudiRSSMainWindow::OnListBoxCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
    case IDC_FEED_LIST:
    {
        OnProcessFeedList(hWnd, message, wParam, lParam);
        break;
    }

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void RudiRSSMainWindow::OnProcessFeedList(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case LBN_SELCHANGE:
    {
        int itemIdx = (int)SendMessage(m_feedListBox.m_hWnd, LB_GETCURSEL, 0, 0);
        long long feedId = (long long)SendMessage(m_feedListBox.m_hWnd, LB_GETITEMDATA, itemIdx, 0);

        std::string guid;
        m_rudiRSSClient.QueryFeed(feedId, [&](const FeedDatabase::Feed& feed) {
            guid = feed.guid;
            });

        SendMessage(m_feedTitleListView.m_hWnd, LVM_DELETEALLITEMS, 0, 0);

        m_rudiRSSClient.QueryFeedData(guid, [&](const FeedDatabase::FeedData& feedData) {
            InsertIntoFeedTitleListView(feedData);
            });
    }
    break;

    default:
        break;
    }
}

LRESULT RudiRSSMainWindow::OnProcessListViewCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
    case IDC_FEED_TITLE_LIST_VIEW:
    {
        OnProcessFeedTitleListView(hWnd, message, wParam, lParam);
        break;
    }

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void RudiRSSMainWindow::OnProcessFeedTitleListView(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPNMITEMACTIVATE itemActivate = (LPNMITEMACTIVATE)lParam;
    switch (itemActivate->hdr.code)
    {
    case NM_CLICK:
    {
        if (InterlockedOr(reinterpret_cast<LONG*>(&m_initViewer), 0))
        {
            long long feedDataId = static_cast<long long>(GetLParamFromListView(itemActivate));
            m_rudiRSSClient.QueryFeedData(feedDataId, [&](const FeedDatabase::FeedData& feedData) {
                std::wstring link;
                FeedCommon::ConvertStringToWideString(feedData.link, link);
                m_viewer.Navigate(link);
                });
        }
    }
    break;

    default:
        break;
    }
}

bool RudiRSSMainWindow::FeedIdExistInSet(long long feedid)
{
    ATL::CComCritSecLock lock(m_feedListLock);
    return m_feedIdSet.end() != m_feedIdSet.find(feedid);
}

void RudiRSSMainWindow::InsertFeedIdIntoSet(long long feedid)
{
    ATL::CComCritSecLock lock(m_feedListLock);
    m_feedIdSet.insert(feedid);
}

void RudiRSSMainWindow::ClearFeedIdSet()
{
    ATL::CComCritSecLock lock(m_feedListLock);
    m_feedIdSet.clear();
}
