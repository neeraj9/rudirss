#include "Viewer.h"

#include <string>

Viewer::Viewer() : m_hWndParent{ nullptr }, m_rc{}
{

}

Viewer::~Viewer()
{
    Destroy();
}

HRESULT Viewer::Initialize(HWND hWnd, const RECT &rc, FN_ON_WEBVIEW_READY fnOnWebViewReady)
{
    m_hWndParent = hWnd;
    m_fnOnWebViewReady = fnOnWebViewReady;
    memcpy(&m_rc, &rc, sizeof(rc));
    HRESULT result = CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [&](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

                // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
                env->CreateCoreWebView2Controller(m_hWndParent, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [&](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                        if (controller != nullptr) {
                            m_webviewController = controller;
                            m_webviewController->get_CoreWebView2(&m_webview);
                        }

                        // Add a few settings for the webview
                        // The demo step is redundant since the values are the default settings
                        wil::com_ptr<ICoreWebView2Settings> settings;
                        m_webview->get_Settings(&settings);
                        settings->put_IsScriptEnabled(TRUE);
                        settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                        settings->put_IsWebMessageEnabled(TRUE);

                        // Resize WebView to fit the bounds of the parent window
                        m_webviewController->put_Bounds(m_rc);

                        // <NavigationEvents>
                        // Step 4 - Navigation events
                        // register an ICoreWebView2NavigationStartingEventHandler to cancel any non-https navigation
                        EventRegistrationToken token;
                        m_webview->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
                            [](ICoreWebView2* webview, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
                                wil::unique_cotaskmem_string uri;
                                args->get_Uri(&uri);
                                std::wstring source(uri.get());
                                if (source.substr(0, 5) != L"https") {
                                    args->put_Cancel(true);
                                }
                                return S_OK;
                            }).Get(), &token);
                        // </NavigationEvents>

                        if (m_fnOnWebViewReady)
                            m_fnOnWebViewReady();

                        return S_OK;
                    }).Get());
                return S_OK;
            }).Get());

    return result;
}

HRESULT Viewer::Navigate(const std::wstring& uri)
{
    HRESULT result = E_FAIL;
    if (m_webview)
        result = m_webview->Navigate(uri.c_str());

    return result;
}

HRESULT Viewer::MoveWindow(const RECT& rc)
{
    HRESULT result = E_FAIL;
    if (m_webviewController)
        result = m_webviewController->put_Bounds(rc);

    return result;
}

void Viewer::Destroy()
{
    if (m_webview)
    {
        m_webview->Stop();
        m_webview.reset();
    }

    if (m_webviewController)
    {
        m_webviewController->Close();
        m_webviewController.reset();
    }
}
