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
    LONG width = rc.right - rc.left;
    LONG height = rc.bottom - rc.top;

    m_feedListBox.Attach(CreateWindowExW(WS_EX_CLIENTEDGE,
        L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | LBS_NOTIFY,
        0, 0, 300, height, m_hWnd, (HMENU)IDC_FEED_LIST, m_hInstance, NULL));

    RECT viewerRect{};
    memcpy(&viewerRect, &rc, sizeof(rc));
    viewerRect.left = 310;
    HRESULT result = m_viewer.Initialize(m_hWnd, viewerRect, [&]() {
        InterlockedExchange(reinterpret_cast<LONG*>(&m_initViewer), TRUE);
        });

    std::vector<std::wstring> text = { L"https://www.bing.com/", L"https://github.com/FarGroup/FarManager"};
    for (const auto& item : text)
    {
        int pos = (int)SendMessage(m_feedListBox.m_hWnd, LB_ADDSTRING, 0,
            (LPARAM)item.c_str());
    }
    SendMessage(m_feedListBox.m_hWnd, LB_SETCURSEL, 0, 0);
}

void RudiRSSMainWindow::UpdateControl()
{
    if (!m_hWnd)
        return;

    RECT rc{};
    GetClientRect(m_hWnd, &rc);
    LONG width = rc.right - rc.left;
    LONG height = rc.bottom - rc.top;

    if (m_feedListBox.m_hWnd)
    {
        MoveWindow(m_feedListBox.m_hWnd, 0, 0, 300, height, FALSE);
    }

    if (InterlockedOr(reinterpret_cast<LONG*>(&m_initViewer), 0))
    {
        rc.left = 310;
        rc.top = 0;
        m_viewer.MoveWindow(rc);
    }
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
