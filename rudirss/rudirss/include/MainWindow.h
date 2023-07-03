#pragma once

#include "WindowHandle.h"

#include <functional>

class MainWindow : public WindowHandle
{
protected:
    HINSTANCE m_hInstance;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
    MainWindow();
    virtual ~MainWindow();

    virtual bool Initialize(HINSTANCE hInstance);
    virtual HWND Create() = 0;
    virtual void OnRegister(WNDCLASSEXW& wcex) = 0;
    virtual LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
    virtual WPARAM MessageLoop();
};
