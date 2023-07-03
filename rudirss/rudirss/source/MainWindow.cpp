#include "MainWindow.h"

MainWindow::MainWindow()
{

}

MainWindow::~MainWindow()
{

}

bool MainWindow::Initialize(FN_ON_REGISTER fnOnRegister, FN_CREATE_WINDOW fnCreateWindow, FN_ON_PROCESS_MESSAGE fnOnProcessMessage)
{
    if (!fnOnRegister 
        || !fnCreateWindow
        || !fnOnProcessMessage)
        return false;

    WNDCLASSEXW wcex{};
    fnOnRegister(wcex);
    if (0 == RegisterClassExW(&wcex))
        return false;

    m_fnOnProcessMessage = fnOnProcessMessage;

    m_hWnd = fnCreateWindow();
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
        return pThis->m_fnOnProcessMessage(hWnd, message, wParam, lParam);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
