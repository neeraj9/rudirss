#pragma once

#include "WindowHandle.h"

#include <functional>

class MainWindow : public WindowHandle
{
public:
    MainWindow();
    virtual ~MainWindow();

    using FN_ON_REGISTER = std::function<void(WNDCLASSEXW&)>;
    using FN_ON_PROCESS_MESSAGE = std::function<LRESULT(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)>;
    using FN_CREATE_WINDOW = std::function<HWND()>;
    virtual bool Initialize(FN_ON_REGISTER fnOnRegister, FN_CREATE_WINDOW fnCreateWindow, FN_ON_PROCESS_MESSAGE fnOnProcessMessage);

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual WPARAM MessageLoop();

protected:
    FN_ON_PROCESS_MESSAGE m_fnOnProcessMessage;
};
