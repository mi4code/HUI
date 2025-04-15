/*

home: <https://developer.microsoft.com/en-us/microsoft-edge/webview2/>
docs: <https://learn.microsoft.com/cs-cz/microsoft-edge/webview2/samples/webview2apissample>
example: <https://github.com/MicrosoftEdge/WebView2Samples/blob/main/GettingStartedGuides/Win32_GettingStarted/HelloWebView.cpp>
other example: <https://github.com/MicrosoftEdge/WebView2Samples/tree/main/SampleApps/WebView2APISample>

similar issue maybe: <https://stackoverflow.com/questions/77465466/webview2-h-not-found-with-msvc-on-github-actions>

seems like there is no way to compile this in msys2 mingw -- tried with following AI generated code, but i dont have webview2.h

*/


#include <windows.h>
#include <webview2.h>
#include <string>

// Global variables
HWND g_mainWindow = nullptr;
ICoreWebView2Controller* webViewController = nullptr;
ICoreWebView2* webViewWindow = nullptr;

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitializeWebView2(HWND hwnd);
void CleanupWebView();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"WebView2App";
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    g_mainWindow = CreateWindowEx(
        0,
        CLASS_NAME,
        L"WebView2 Simple App",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (g_mainWindow == nullptr)
    {
        return 0;
    }

    ShowWindow(g_mainWindow, nCmdShow);

    InitializeWebView2(g_mainWindow);

    MSG msg = { };
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CleanupWebView();
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
        if (webViewController != nullptr)
        {
            RECT bounds;
            GetClientRect(hwnd, &bounds);
            webViewController->put_Bounds(bounds);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void InitializeWebView2(HWND hwnd)
{
    // Create WebView2 environment
    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hwnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
            {
                if (env)
                {
                    // Create WebView2 controller
                    env->CreateCoreWebView2Controller(hwnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [hwnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                        {
                            if (controller != nullptr)
                            {
                                webViewController = controller;
                                webViewController->get_CoreWebView2(&webViewWindow);
                            }

                            // Resize WebView to fit the window
                            RECT bounds;
                            GetClientRect(hwnd, &bounds);
                            webViewController->put_Bounds(bounds);

                            // Load a local HTML file
                            std::wstring htmlFilePath = L"file:///" + std::wstring(L"example.html");
                            webViewWindow->Navigate(htmlFilePath.c_str());

                            return S_OK;
                        }).Get());
                }
                return S_OK;
            }).Get());
}

void CleanupWebView()
{
    if (webViewController != nullptr)
    {
        webViewController->Release();
        webViewController = nullptr;
    }

    if (webViewWindow != nullptr)
    {
        webViewWindow->Release();
        webViewWindow = nullptr;
    }
}
