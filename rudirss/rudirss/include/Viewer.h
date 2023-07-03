#pragma once

#include "WindowHandle.h"

#include <functional>
#include <string>

#include <wrl.h>
#include <wil/com.h>
#include "WebView2.h"

using namespace Microsoft::WRL;

class Viewer : public WindowHandle
{
protected:
    wil::com_ptr<ICoreWebView2Controller> m_webviewController;
    wil::com_ptr<ICoreWebView2> m_webview;
    HWND m_hWndParent;
    RECT m_rc;

    using FN_ON_WEBVIEW_READY = std::function<void()>;
    FN_ON_WEBVIEW_READY m_fnOnWebViewReady;

public:
    Viewer();
    virtual ~Viewer();

    HRESULT Initialize(HWND hWnd, const RECT &rc, FN_ON_WEBVIEW_READY fnOnWebViewReady);
    HRESULT Navigate(const std::wstring &uri);
    HRESULT MoveWindow(const RECT &rc);
};
