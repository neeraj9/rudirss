#include "RudiRSSMainWindow.h"
#include "Resource.h"

#include <format>
#include <vector>

RudiRSSMainWindow::RudiRSSMainWindow() : m_initViewer{ FALSE }
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
        result = true;
    }

    return result;
}

LRESULT RudiRSSMainWindow::OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        return OnCommand(hWnd, message, wParam, lParam);
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
    RECT rc{};
    GetClientRect(m_hWnd, &rc);
    LONG height = rc.bottom - rc.top;
    LONG viewerX = 0;

    m_feedListBox.Attach(CreateWindowExW(WS_EX_STATICEDGE,//WS_EX_CLIENTEDGE,
        L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | LBS_NOTIFY,
        0, 0, 300, height, m_hWnd, (HMENU)IDC_FEED_LIST, m_hInstance, NULL));
    GetClientRect(m_feedListBox.m_hWnd, &rc);
    viewerX += rc.right - rc.left;

    m_feedTitleListBox.Attach(CreateWindowExW(WS_EX_STATICEDGE,//WS_EX_CLIENTEDGE,
        L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | LBS_NOTIFY,
        rc.right + 1, 0, 300, height, m_hWnd, (HMENU)IDC_FEED_TITLE_LIST, m_hInstance, NULL));
    GetClientRect(m_feedTitleListBox.m_hWnd, &rc);
    viewerX += rc.right - rc.left;

    RECT viewerRect{};
    GetClientRect(m_hWnd, &viewerRect);
    viewerRect.left = viewerX + 1;
    HRESULT result = m_viewer.Initialize(m_hWnd, viewerRect, [&]() {
        InterlockedExchange(reinterpret_cast<LONG*>(&m_initViewer), TRUE);
        });
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

        if (!m_feedTitleListBox.m_hWnd)
            break;

        MoveWindow(m_feedTitleListBox.m_hWnd, rc.right + 1, 0, 300, height, FALSE);
        GetClientRect(m_feedTitleListBox.m_hWnd, &rc);
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

LRESULT RudiRSSMainWindow::OnCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
    case IDC_FEED_LIST:
    {
        switch (HIWORD(wParam))
        {
        case LBN_SELCHANGE:
        {
            if (InterlockedOr(reinterpret_cast<LONG*>(&m_initViewer), 0))
            {
                int itemIdx = (int)SendMessage(m_feedListBox.m_hWnd, LB_GETCURSEL, 0, 0);

                std::vector<WCHAR> sel(512, 0);
                auto len = SendMessage(m_feedListBox.m_hWnd, LB_GETTEXT, itemIdx, (LPARAM)sel.data());
                std::wstring selItem(sel.data(), len);
                m_viewer.Navigate(selItem);
            }
        }
        break;

        default:
            break;
        }

        break;
    }

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
