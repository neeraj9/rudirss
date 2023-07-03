// rudirss.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "rudirss.h"
#include "MainWindow.h"

#include <string>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    WCHAR title[MAX_PATH]{};
    WCHAR windowClass[MAX_PATH]{};
    LoadStringW(hInstance, IDS_APP_TITLE, title, _countof(title));
    LoadStringW(hInstance, IDC_RUDIRSS, windowClass, _countof(windowClass));

    MainWindow mainWindow;
    mainWindow.Initialize([&](WNDCLASSEXW& wcex) {
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = MainWindow::WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RUDIRSS));
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        //wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_RUDIRSS);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = windowClass;
        wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
        }, [&]() -> HWND {
            HWND hWnd = CreateWindowW(windowClass, title, WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

            ShowWindow(hWnd, nCmdShow);
            UpdateWindow(hWnd);
            return hWnd;
        }, [&](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT {
            switch (message)
            {
            case WM_COMMAND:
            {
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
            case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                // TODO: Add any drawing code that uses hdc here...
                EndPaint(hWnd, &ps);
            }
            break;
            case WM_DESTROY:
                PostQuitMessage(0);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            return 0;
        });

    return static_cast<int>(mainWindow.MessageLoop());
}