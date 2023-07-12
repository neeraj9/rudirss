#include "RudiRSSMainWindow.h"
#include "Resource.h"
#include "FeedBase.h"

#include <format>
#include <vector>

RudiRSSMainWindow::RudiRSSMainWindow() : m_initViewer{ FALSE }, m_font{ nullptr }
{
    m_feedListViewWidth = 250;
    m_feedTitleListTitleColumnWidth = 250;
    m_feedTitleListUpdatedWidth = 150;
    m_feedTitleListWidth = m_feedTitleListTitleColumnWidth + m_feedTitleListUpdatedWidth;

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
                InsertIntoFeedListView(feed);
                });

            m_rudiRSSClient.StartRefreshFeedTimer(0, 1800 * 1000, [&](const FeedDatabase::FeedConsumptionUnit& consumptionUnit) {
                if (FeedDatabase::FeedConsumptionUnit::OperationType::NOTIFY_INSERTION_COMPLETE == consumptionUnit.opType)
                {
                    if (!FeedIdExistInSet(consumptionUnit.feed.feedid))
                        InsertIntoFeedListView(consumptionUnit.feed);
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

    InitFeedListView(0, 0, m_feedListViewWidth, height);
    GetClientRect(m_feedListView.m_hWnd, &rc);
    viewerX += rc.right - rc.left;

    int titleColWidth = m_feedTitleListTitleColumnWidth;
    int updatedColWidth = m_feedTitleListUpdatedWidth;
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

void RudiRSSMainWindow::InitFeedListView(int x, int y, int width, int height)
{
    DWORD dwStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOCOLUMNHEADER;
    m_feedListView.Attach(CreateWindowEx(WS_EX_STATICEDGE, WC_LISTVIEW, nullptr, dwStyle, x, y,
        width, height, m_hWnd, (HMENU)IDC_FEED_LIST_VIEW, m_hInstance, nullptr));
    SendMessage(m_feedListView.m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT); // Set extended style
    SendMessage(m_feedListView.m_hWnd, WM_SETFONT, (WPARAM)m_font, TRUE);

    LV_COLUMN   lvColumn{};
    lvColumn.mask = LVCF_WIDTH | LVCF_TEXT;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.cx = width;
    lvColumn.pszText = const_cast<wchar_t*>(L"Subscribed");
    SendMessage(m_feedListView.m_hWnd, LVM_INSERTCOLUMN, 0, (LPARAM)&lvColumn);

    // Initialize a 'All feeds' item so that it can display all feeds from database
    LVITEM lvItem{};
    std::wstring text;
    lvItem.pszText = const_cast<wchar_t*>(L"All feeds");
    lvItem.iItem = 0;
    lvItem.iSubItem = 0;
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam = (LPARAM)FeedDatabase::INVALID_FEEDDATA_ID;
    SendMessage(m_feedListView.m_hWnd, LVM_INSERTITEM, 0, (LPARAM)&lvItem);
}

void RudiRSSMainWindow::InsertIntoFeedListView(const FeedDatabase::Feed& feed)
{
    LVITEM lvItem{};
    int col = 0;
    std::wstring text;
    FeedCommon::ConvertStringToWideString(feed.title, text);
    lvItem.pszText = text.data();
    lvItem.iItem = static_cast<int>(SendMessage(m_feedListView.m_hWnd, LVM_GETITEMCOUNT, 0, 0));
    lvItem.iSubItem = col++;
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam = (LPARAM)feed.feedid;
    SendMessage(m_feedListView.m_hWnd, LVM_INSERTITEM, 0, (LPARAM)&lvItem);
}

void RudiRSSMainWindow::InitFeedTitleListView(int x, int y, int width, int height, int titleColWidth, int updatedColWidth)
{
    DWORD dwStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT;
    m_feedTitleListView.Attach(CreateWindowEx(WS_EX_STATICEDGE, WC_LISTVIEW, nullptr, dwStyle, x, y,
        titleColWidth + updatedColWidth, height, m_hWnd, (HMENU)IDC_FEED_TITLE_LIST_VIEW, m_hInstance, nullptr));
    SendMessage(m_feedTitleListView.m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT); // Set extended style
    SendMessage(m_feedTitleListView.m_hWnd, WM_SETFONT, (WPARAM)m_font, TRUE);

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
    LVITEM lvItem{};
    int col = 0;
    std::wstring text;
    FeedCommon::ConvertStringToWideString(feedData.title, text);
    text = GetReadStateSymbol(feedData.read) + text;
    lvItem.pszText = text.data();
    lvItem.iItem = static_cast<int>(SendMessage(m_feedTitleListView.m_hWnd, LVM_GETITEMCOUNT, 0, 0));
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
    int selItem = static_cast<int>(SendMessage(m_feedTitleListView.m_hWnd, LVM_GETNEXTITEM, -1, LVNI_SELECTED));
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

std::wstring RudiRSSMainWindow::GetReadStateSymbol(long long read)
{
    return 0 != read ? L"[X] " : L"[ ] ";
}

void RudiRSSMainWindow::MarkFeedDataAsReadOrUnRead(LPNMITEMACTIVATE activateItem, const std::wstring& title, long long read)
{
    std::wstring text = GetReadStateSymbol(read) + title;;
    LVITEM lvItem{};
    lvItem.iItem = activateItem->iItem;
    lvItem.iSubItem = activateItem->iSubItem;
    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = text.data();
    SendMessage(activateItem->hdr.hwndFrom, LVM_SETITEM, 0, (LPARAM)&lvItem);
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

        MoveWindow(m_feedListView.m_hWnd, 0, 0, m_feedListViewWidth, height, FALSE);
        GetClientRect(m_feedListView.m_hWnd, &rc);
        viewerX += rc.right - rc.left;

        if (!m_feedTitleListView.m_hWnd)
            break;

        MoveWindow(m_feedTitleListView.m_hWnd, rc.right + 1, 0, m_feedTitleListWidth, height, FALSE);
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

LRESULT RudiRSSMainWindow::OnProcessListViewCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
    case IDC_FEED_LIST_VIEW:
    {
        return OnProcessFeedListView(hWnd, message, wParam, lParam);
    }

    case IDC_FEED_TITLE_LIST_VIEW:
    {
        return OnProcessFeedTitleListView(hWnd, message, wParam, lParam);
    }

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT RudiRSSMainWindow::OnProcessFeedListView(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPNMITEMACTIVATE itemActivate = (LPNMITEMACTIVATE)lParam;
    switch (itemActivate->hdr.code)
    {
    case NM_CLICK:
    {
        long long feedId = static_cast<long long>(GetLParamFromListView(itemActivate));
        SendMessage(m_feedTitleListView.m_hWnd, LVM_DELETEALLITEMS, 0, 0);
        if (FeedDatabase::INVALID_FEEDDATA_ID != feedId)
        {
            m_rudiRSSClient.QueryFeedDataOrderByTimestamp(feedId, [&](const FeedDatabase::FeedData& feedData) {
                InsertIntoFeedTitleListView(feedData);
                });
        }
        else
        {
            m_rudiRSSClient.QueryAllFeedDataOrderByTimestamp([&](const FeedDatabase::FeedData& feedData) {
                InsertIntoFeedTitleListView(feedData);
                });
        }
    }
    break;

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT RudiRSSMainWindow::OnProcessFeedTitleListView(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPNMITEMACTIVATE itemActivate = (LPNMITEMACTIVATE)lParam;
    switch (itemActivate->hdr.code)
    {
    case NM_CLICK:
    {
        if (InterlockedOr(reinterpret_cast<LONG*>(&m_initViewer), 0))
        {
            long long feedDataId = static_cast<long long>(GetLParamFromListView(itemActivate));
            std::wstring title;
            m_rudiRSSClient.QueryFeedDataByFeedDataId(feedDataId, [&](const FeedDatabase::FeedData& feedData) {
                FeedCommon::ConvertStringToWideString(feedData.title, title);
                std::wstring link;
                FeedCommon::ConvertStringToWideString(feedData.link, link);
                m_viewer.Navigate(link);
                });
            m_rudiRSSClient.UpdateFeedDataReadColumn(feedDataId, static_cast<long long>(true));
            MarkFeedDataAsReadOrUnRead(itemActivate, title, static_cast<long long>(true));
        }
    }
    break;

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
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
