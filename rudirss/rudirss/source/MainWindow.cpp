#include "MainWindow.h"

MainWindow::MainWindow() : m_hInstance{ nullptr }
{

}

MainWindow::~MainWindow()
{

}

bool MainWindow::Initialize(HINSTANCE hInstance)
{
    m_hInstance = hInstance;

    WNDCLASSEXW wcex{};
    OnRegister(wcex);
    if (0 == RegisterClassExW(&wcex))
        return false;

    m_hWnd = Create();
    if (!m_hWnd)
        return false;

    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    return true;
}

WPARAM MainWindow::MessageLoop()
{
    MSG msg{};

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK MainWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LONG_PTR param = GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (param)
    {
        auto pThis = reinterpret_cast<MainWindow*>(param);
        return pThis->OnProcessMessage(hWnd, message, wParam, lParam);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
