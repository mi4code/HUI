#define TEST12

#if defined(TEST1)
#include <windows.h>
#include <iostream>
#include <string>

// Global variables
HWND hwndMain, hwndFFmpeg, hwndNotepad;

// Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void EmbedWindow(HWND parent, HWND child, RECT rect);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    MSG msg;
    WNDCLASS wc = {0};

    // Register the window class
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MainAppWindow";

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Failed to register window class", "Error", MB_ICONERROR);
        return 1;
    }

    // Create the main window
    hwndMain = CreateWindow("MainAppWindow", "Embedded Applications", WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    if (!hwndMain) {
        MessageBox(NULL, "Failed to create window", "Error", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

    // Start FFmpeg and Notepad processes
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    // Launch Notepad
    if (CreateProcess(NULL, "notepad.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        // Find the Notepad window
        hwndNotepad = FindWindow("li-ai", NULL);
        if (hwndNotepad) {
            RECT rect = {0, 0, 400, 300};  // Define the position and size
            EmbedWindow(hwndMain, hwndNotepad, rect);
        } else {
            MessageBox(NULL, "Failed to find Notepad window", "Error", MB_ICONERROR);
        }
    } else {
        MessageBox(NULL, "Failed to launch Notepad", "Error", MB_ICONERROR);
    }

    // Launch FFmpeg (example command, FFmpeg usually runs in console mode)
    if (CreateProcess(NULL, "ffmpeg.exe -version", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        // FFmpeg does not create a window by default; this part is illustrative
        hwndFFmpeg = FindWindow(NULL, "FFmpeg");  // Adjust as needed for FFmpeg
        if (hwndFFmpeg) {
            RECT rect = {0, 300, 400, 600};  // Define the position and size
            EmbedWindow(hwndMain, hwndFFmpeg, rect);
        } else {
            MessageBox(NULL, "Failed to find FFmpeg window", "Error", MB_ICONERROR);
        }
    } else {
        MessageBox(NULL, "Failed to launch FFmpeg", "Error", MB_ICONERROR);
    }

    // Message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void EmbedWindow(HWND parent, HWND child, RECT rect) {
    SetParent(child, parent);
    SetWindowLong(child, GWL_STYLE, GetWindowLong(child, GWL_STYLE) & ~WS_POPUP | WS_CHILD);
    SetWindowPos(child, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
                 SWP_NOZORDER | SWP_FRAMECHANGED);
}
#endif

#if defined(TEST2)

#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <string>

// Global variables
HINSTANCE hInst;
HWND hwndFFPlay;
HWND hwndNotepad;

// Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height);
HWND StartProcessAndGetWindow(LPCTSTR exeName);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine, int       nCmdShow)
{
    MSG msg;
    WNDCLASS wc = {0};
    HWND hwndParent;

    // Register window class
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = _T("ParentWindowClass");
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc))
        return 1;

    hInst = hInstance;

    // Create the parent window
    hwndParent = CreateWindow(wc.lpszClassName, _T("Embedded Applications"),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                              NULL, NULL, hInstance, NULL);

    if (!hwndParent)
        return 1;

    // Start and embed ffplay
    hwndFFPlay = StartProcessAndGetWindow(_T("mspaint.exe"));
    if (hwndFFPlay) {
        EmbedWindow(hwndParent, hwndFFPlay, 0, 0, 400, 600);
    }

    // Start and embed notepad
    hwndNotepad = StartProcessAndGetWindow(_T("notepad.exe"));
    if (hwndNotepad) {
        EmbedWindow(hwndParent, hwndNotepad, 400, 0, 400, 600);
    }

    ShowWindow(hwndParent, nCmdShow);
    UpdateWindow(hwndParent);

    // Message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height)
{
    SetParent(child, parent);
    SetWindowLong(child, GWL_STYLE, WS_CHILD | WS_VISIBLE);
    MoveWindow(child, x, y, width, height, TRUE);
}

HWND StartProcessAndGetWindow(LPCTSTR exeName)
{
    SHELLEXECUTEINFO shExecInfo = {0};
    shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExecInfo.hwnd = NULL;
    shExecInfo.lpVerb = NULL;
    shExecInfo.lpFile = exeName;
    shExecInfo.lpParameters = NULL;
    shExecInfo.lpDirectory = NULL;
    shExecInfo.nShow = SW_SHOW;
    shExecInfo.hInstApp = NULL;

    if (ShellExecuteEx(&shExecInfo) && shExecInfo.hProcess != NULL)
    {
        WaitForInputIdle(shExecInfo.hProcess, INFINITE);

        // Get the window handle of the started process
        HWND hwndProcess = NULL;
        EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
            DWORD pid;
            GetWindowThreadProcessId(hwnd, &pid);
            if (pid == GetProcessId((HANDLE)lParam)) {
                *(HWND*)lParam = hwnd;
                return FALSE;
            }
            return TRUE;
        }, (LPARAM)&hwndProcess);

        CloseHandle(shExecInfo.hProcess);
        return hwndProcess;
    }
    return NULL;
}
#endif

#if defined(TEST3)

#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <string>
#include <vector>

// Global variables
HINSTANCE hInst;
HWND hwndFFPlay;
HWND hwndNotepad;

// Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height);
HWND StartProcessAndGetWindow(LPCTSTR exeName);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine, int       nCmdShow)
{
    MSG msg;
    WNDCLASS wc = {0};
    HWND hwndParent;

    // Register window class
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = _T("ParentWindowClass");
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc))
        return 1;

    hInst = hInstance;

    // Create the parent window
    hwndParent = CreateWindow(wc.lpszClassName, _T("Embedded Applications"),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                              NULL, NULL, hInstance, NULL);

    if (!hwndParent)
        return 1;

    ShowWindow(hwndParent, nCmdShow);
    UpdateWindow(hwndParent);

    // Start and embed ffplay
    hwndFFPlay = StartProcessAndGetWindow(_T("mspaint.exe"));
    if (hwndFFPlay) {
        EmbedWindow(hwndParent, hwndFFPlay, 0, 0, 400, 600);
    }

    // Start and embed notepad
    hwndNotepad = StartProcessAndGetWindow(_T("notepad.exe"));
    if (hwndNotepad) {
        EmbedWindow(hwndParent, hwndNotepad, 400, 0, 400, 600);
    }

    // Message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height)
{
    SetParent(child, parent);
    SetWindowLong(child, GWL_STYLE, WS_CHILD | WS_VISIBLE);
    MoveWindow(child, x, y, width, height, TRUE);
}

HWND StartProcessAndGetWindow(LPCTSTR exeName)
{
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;
    HWND hwndProcess = NULL;

    if (CreateProcess(NULL, const_cast<LPTSTR>(exeName), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        WaitForInputIdle(pi.hProcess, INFINITE);

        // Get the window handle of the started process
        EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
            DWORD pid;
            GetWindowThreadProcessId(hwnd, &pid);
            if (pid == GetProcessId((HANDLE)lParam)) {
                *(HWND*)lParam = hwnd;
                return FALSE;
            }
            return TRUE;
        }, (LPARAM)&hwndProcess);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    return hwndProcess;
}


#endif

#if defined(TEST4) // broken but at least something
#include <windows.h>
#include <tchar.h>
#include <string>

// Global variables
HINSTANCE hInst;
HWND hwndFFPlay;
HWND hwndNotepad;

// Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height);
HWND StartProcessAndGetWindow(LPCTSTR exeName);
BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);
HWND FindMainWindow(DWORD processID);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine, int       nCmdShow)
{
    MSG msg;
    WNDCLASS wc = {0};
    HWND hwndParent;

    // Register window class
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = _T("ParentWindowClass");
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc))
        return 1;

    hInst = hInstance;

    // Create the parent window
    hwndParent = CreateWindow(wc.lpszClassName, _T("Embedded Applications"),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                              NULL, NULL, hInstance, NULL);

    if (!hwndParent)
        return 1;

    ShowWindow(hwndParent, nCmdShow);
    UpdateWindow(hwndParent);

    // Start and embed ffplay
    hwndFFPlay = StartProcessAndGetWindow(_T("mspaint.exe"));
    if (hwndFFPlay) {
        EmbedWindow(hwndParent, hwndFFPlay, 0, 0, 400, 600);
    }

    // Start and embed notepad
    hwndNotepad = StartProcessAndGetWindow(_T("notepad.exe"));
    if (hwndNotepad) {
        EmbedWindow(hwndParent, hwndNotepad, 400, 0, 400, 600);
    }

    // Message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height)
{
    SetParent(child, parent);
    SetWindowLong(child, GWL_STYLE, WS_CHILD | WS_VISIBLE);
    MoveWindow(child, x, y, width, height, TRUE);
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    struct HandleData {
        DWORD processID;
        HWND bestHandle;
    };

    HandleData& data = *(HandleData*)lParam;
    DWORD processID = 0;
    GetWindowThreadProcessId(handle, &processID);
    if (data.processID == processID && GetWindow(handle, GW_OWNER) == 0 && IsWindowVisible(handle))
    {
        data.bestHandle = handle;
        return FALSE;
    }
    return TRUE;
}

HWND FindMainWindow(DWORD processID)
{
    struct HandleData {
        DWORD processID;
        HWND bestHandle;
    };

    HandleData data = { processID, 0 };
    EnumWindows(EnumWindowsCallback, (LPARAM)&data);
    return data.bestHandle;
}

HWND StartProcessAndGetWindow(LPCTSTR exeName)
{
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcess(NULL, const_cast<LPTSTR>(exeName), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        WaitForInputIdle(pi.hProcess, INFINITE);
        HWND hwnd = FindMainWindow(pi.dwProcessId);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return hwnd;
    }
    return NULL;
}


#endif

#if defined(TEST5) // nested wm
#include <windows.h>
#include <tchar.h>
#include <string>

// Global variables
HINSTANCE hInst;
HWND hwndFFPlay;
HWND hwndNotepad;

// Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height);
HWND StartProcessAndGetWindow(LPCTSTR exeName);
BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);
HWND FindMainWindow(DWORD processID);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine, int       nCmdShow)
{
    MSG msg;
    WNDCLASS wc = {0};
    HWND hwndParent;

    // Register window class
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = _T("ParentWindowClass");
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc))
        return 1;

    hInst = hInstance;

    // Create the parent window
    hwndParent = CreateWindow(wc.lpszClassName, _T("Embedded Applications"),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                              NULL, NULL, hInstance, NULL);

    if (!hwndParent)
        return 1;

    ShowWindow(hwndParent, nCmdShow);
    UpdateWindow(hwndParent);

    // Start and embed ffplay
    //hwndFFPlay = StartProcessAndGetWindow(_T("notepad.exe"));
    hwndFFPlay = StartProcessAndGetWindow(_T("mspaint.exe"));
    //hwndFFPlay = StartProcessAndGetWindow(_T("ffplay C:/msys64/mingw64/home/demo.mkv"));
    //hwndFFPlay = StartProcessAndGetWindow(_T("C:/Apps/PortableApps/7-Zip File Manager/7zFM.exe"));
    if (hwndFFPlay) {
        EmbedWindow(hwndParent, hwndFFPlay, 0, 0, 400, 600);
    }

    // Start and embed notepad
    hwndNotepad = StartProcessAndGetWindow(_T("notepad.exe"));
    if (hwndNotepad) {
        EmbedWindow(hwndParent, hwndNotepad, 400, 0, 400, 600);
    }

    // Message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        {
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            int halfWidth = rcClient.right / 2;

            if (hwndFFPlay) {
                MoveWindow(hwndFFPlay, 0, 0, halfWidth, rcClient.bottom, TRUE);
            }
            if (hwndNotepad) {
                MoveWindow(hwndNotepad, halfWidth, 0, halfWidth, rcClient.bottom, TRUE);
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height)
{
    SetParent(child, parent);
    LONG style = GetWindowLong(child, GWL_STYLE);
    style &= ~WS_POPUP;
    style |= WS_CHILD;
    SetWindowLong(child, GWL_STYLE, style);

    SetWindowPos(child, HWND_TOP, x, y, width, height, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    struct HandleData {
        DWORD processID;
        HWND bestHandle;
    };

    HandleData& data = *(HandleData*)lParam;
    DWORD processID = 0;
    GetWindowThreadProcessId(handle, &processID);
    if (data.processID == processID && GetWindow(handle, GW_OWNER) == 0 && IsWindowVisible(handle))
    {
        data.bestHandle = handle;
        return FALSE;
    }
    return TRUE;
}

HWND FindMainWindow(DWORD processID)
{
    struct HandleData {
        DWORD processID;
        HWND bestHandle;
    };

    HandleData data = { processID, 0 };
    EnumWindows(EnumWindowsCallback, (LPARAM)&data);
    return data.bestHandle;
}

HWND StartProcessAndGetWindow(LPCTSTR exeName)
{
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcess(NULL, const_cast<LPTSTR>(exeName), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        WaitForInputIdle(pi.hProcess, INFINITE);
        HWND hwnd = FindMainWindow(pi.dwProcessId);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return hwnd;
    }
    return NULL;
}

#endif

#if defined(TEST6) // something little broken

#include <windows.h>
#include <tchar.h>
#include <string>

// Global variables
HINSTANCE hInst;
HWND hwndMSPaint;
HWND hwndNotepad;

// Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height);
HWND StartProcessAndGetWindow(LPCTSTR exeName);
BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);
HWND FindMainWindow(DWORD processID);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine, int       nCmdShow)
{
    MSG msg;
    WNDCLASS wc = {0};
    HWND hwndParent;

    // Register window class
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = _T("ParentWindowClass");
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc))
        return 1;

    hInst = hInstance;

    // Create the parent window
    hwndParent = CreateWindow(wc.lpszClassName, _T("Embedded Applications"),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                              NULL, NULL, hInstance, NULL);

    if (!hwndParent)
        return 1;

    ShowWindow(hwndParent, nCmdShow);
    UpdateWindow(hwndParent);

    // Start and embed mspaint
    hwndMSPaint = StartProcessAndGetWindow(_T("mspaint.exe"));
    if (hwndMSPaint) {
        EmbedWindow(hwndParent, hwndMSPaint, 0, 0, 400, 600);
    }

    // Start and embed notepad
    hwndNotepad = StartProcessAndGetWindow(_T("notepad.exe"));
    if (hwndNotepad) {
        EmbedWindow(hwndParent, hwndNotepad, 400, 0, 400, 600);
    }

    // Message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        {
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            int halfWidth = rcClient.right / 2;

            if (hwndMSPaint) {
                MoveWindow(hwndMSPaint, 0, 0, halfWidth, rcClient.bottom, TRUE);
            }
            if (hwndNotepad) {
                MoveWindow(hwndNotepad, halfWidth, 0, halfWidth, rcClient.bottom, TRUE);
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height)
{
    // Remove all styles that allow the child window to be resized, moved, or decorated
    SetParent(child, parent);
    LONG style = GetWindowLong(child, GWL_STYLE);
    style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    style |= WS_CHILD;
    SetWindowLong(child, GWL_STYLE, style);
    
    // Apply the new window style and size
    SetWindowPos(child, NULL, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    struct HandleData {
        DWORD processID;
        HWND bestHandle;
    };

    HandleData& data = *(HandleData*)lParam;
    DWORD processID = 0;
    GetWindowThreadProcessId(handle, &processID);
    if (data.processID == processID && GetWindow(handle, GW_OWNER) == 0 && IsWindowVisible(handle))
    {
        data.bestHandle = handle;
        return FALSE;
    }
    return TRUE;
}

HWND FindMainWindow(DWORD processID)
{
    struct HandleData {
        DWORD processID;
        HWND bestHandle;
    };

    HandleData data = { processID, 0 };
    EnumWindows(EnumWindowsCallback, (LPARAM)&data);
    return data.bestHandle;
}

HWND StartProcessAndGetWindow(LPCTSTR exeName)
{
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcess(NULL, const_cast<LPTSTR>(exeName), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        WaitForInputIdle(pi.hProcess, INFINITE);
        HWND hwnd = FindMainWindow(pi.dwProcessId);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return hwnd;
    }
    return NULL;
}


#endif

#if defined(TEST7) // something little broken + logging
#include <windows.h>
#include <tchar.h>
#include <string>
#include <iostream>

// Global variables
HINSTANCE hInst;
HWND hwndMSPaint;
HWND hwndNotepad;

// Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height);
HWND StartProcessAndGetWindow(LPCTSTR exeName);
BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);
HWND FindMainWindow(DWORD processID);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine, int       nCmdShow)
{
    MSG msg;
    WNDCLASS wc = {0};
    HWND hwndParent;

    // Register window class
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = _T("ParentWindowClass");
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        std::cerr << "Failed to register window class" << std::endl;
        return 1;
    }

    hInst = hInstance;

    // Create the parent window
    hwndParent = CreateWindow(wc.lpszClassName, _T("Embedded Applications"),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                              NULL, NULL, hInstance, NULL);

    if (!hwndParent) {
        std::cerr << "Failed to create parent window" << std::endl;
        return 1;
    }

    ShowWindow(hwndParent, nCmdShow);
    UpdateWindow(hwndParent);

    // Start and embed mspaint
    hwndMSPaint = StartProcessAndGetWindow(_T("mspaint.exe"));
    if (hwndMSPaint) {
        std::cout << "Embedding MSPaint window" << std::endl;
        EmbedWindow(hwndParent, hwndMSPaint, 0, 0, 400, 600);
    } else {
        std::cerr << "Failed to get MSPaint window handle" << std::endl;
    }

    // Start and embed notepad
    hwndNotepad = StartProcessAndGetWindow(_T("notepad.exe"));
    if (hwndNotepad) {
        std::cout << "Embedding Notepad window" << std::endl;
        EmbedWindow(hwndParent, hwndNotepad, 400, 0, 400, 600);
    } else {
        std::cerr << "Failed to get Notepad window handle" << std::endl;
    }

    // Message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        {
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            int halfWidth = rcClient.right / 2;

            std::cout << "Resizing embedded windows" << std::endl;
            if (hwndMSPaint) {
                std::cout << "Resizing MSPaint window" << std::endl;
                MoveWindow(hwndMSPaint, 0, 0, halfWidth, rcClient.bottom, TRUE);
            }
            if (hwndNotepad) {
                std::cout << "Resizing Notepad window" << std::endl;
                MoveWindow(hwndNotepad, halfWidth, 0, halfWidth, rcClient.bottom, TRUE);
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height)
{
    // Remove all styles that allow the child window to be resized, moved, or decorated
    SetParent(child, parent);
    LONG style = GetWindowLong(child, GWL_STYLE);
    style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    style |= WS_CHILD | WS_VISIBLE;
    SetWindowLong(child, GWL_STYLE, style);

    // Make sure the child window can be focused
    LONG exStyle = GetWindowLong(child, GWL_EXSTYLE);
    exStyle &= ~(WS_EX_APPWINDOW | WS_EX_TOOLWINDOW);
    exStyle |= WS_EX_CONTROLPARENT;
    SetWindowLong(child, GWL_EXSTYLE, exStyle);

    // Apply the new window style and size
    SetWindowPos(child, NULL, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

    std::cout << "Window embedded: " << child << std::endl;
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    struct HandleData {
        DWORD processID;
        HWND bestHandle;
    };

    HandleData& data = *(HandleData*)lParam;
    DWORD processID = 0;
    GetWindowThreadProcessId(handle, &processID);
    if (data.processID == processID && GetWindow(handle, GW_OWNER) == 0 && IsWindowVisible(handle))
    {
        data.bestHandle = handle;
        return FALSE;
    }
    return TRUE;
}

HWND FindMainWindow(DWORD processID)
{
    struct HandleData {
        DWORD processID;
        HWND bestHandle;
    };

    HandleData data = { processID, 0 };
    EnumWindows(EnumWindowsCallback, (LPARAM)&data);
    return data.bestHandle;
}

HWND StartProcessAndGetWindow(LPCTSTR exeName)
{
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcess(NULL, const_cast<LPTSTR>(exeName), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        WaitForInputIdle(pi.hProcess, INFINITE);
        HWND hwnd = FindMainWindow(pi.dwProcessId);
        std::cout << "Started process: " << exeName << ", HWND: " << hwnd << std::endl;
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return hwnd;
    }
    std::cerr << "Failed to start process: " << exeName << std::endl;
    return NULL;
}

#endif

#if defined(TEST8) // functional

#include <windows.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include <thread> // For std::this_thread::sleep_for
#include <chrono> // For std::chrono::seconds

// Global variables
HINSTANCE hInst;
HWND hwndMSPaint;
HWND hwndNotepad;

// Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height);
HWND StartProcessAndGetWindow(LPCTSTR exeName);
BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);
HWND FindMainWindow(DWORD processID);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine, int       nCmdShow)
{
    MSG msg;
    WNDCLASS wc = {0};
    HWND hwndParent;
	


    // Register window class
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = _T("ParentWindowClass");
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        std::cerr << "Failed to register window class" << std::endl;
        return 1;
    }

    hInst = hInstance;

    // Create the parent window
    hwndParent = CreateWindow(wc.lpszClassName, _T("Embedded Applications"),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                              NULL, NULL, hInstance, NULL);

    if (!hwndParent) {
        std::cerr << "Failed to create parent window" << std::endl;
        return 1;
    }

    ShowWindow(hwndParent, nCmdShow);
    UpdateWindow(hwndParent);

    // Start and embed mspaint
    hwndMSPaint = StartProcessAndGetWindow(_T("mspaint.exe"));
    if (hwndMSPaint) {
        std::cout << "Embedding MSPaint window" << std::endl;
        EmbedWindow(hwndParent, hwndMSPaint, 0, 0, 400, 600);
    } else {
        std::cerr << "Failed to get MSPaint window handle" << std::endl;
    }

    // Start and embed notepad
    //hwndNotepad = StartProcessAndGetWindow(_T("notepad.exe"));
	hwndNotepad = StartProcessAndGetWindow(_T("ffplay C:/msys64/mingw64/home/demo.mkv"));
    if (hwndNotepad) {
        std::cout << "Embedding Notepad window" << std::endl;
        EmbedWindow(hwndParent, hwndNotepad, 400, 0, 400, 600);
    } else {
        std::cerr << "Failed to get Notepad window handle" << std::endl;
    }

    // Message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }	

    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        {
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            int halfWidth = rcClient.right / 2;

            std::cout << "Resizing embedded windows" << std::endl;
            if (hwndMSPaint) {
                std::cout << "Resizing MSPaint window" << std::endl;
                MoveWindow(hwndMSPaint, 0, 0, halfWidth, rcClient.bottom, TRUE);
            }
            if (hwndNotepad) {
                std::cout << "Resizing Notepad window" << std::endl;
                MoveWindow(hwndNotepad, halfWidth, 0, halfWidth, rcClient.bottom, TRUE);
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height)
{
    // Remove all styles that allow the child window to be resized, moved, or decorated
    SetParent(child, parent);
    LONG style = GetWindowLong(child, GWL_STYLE);
    style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    style |= WS_CHILD | WS_VISIBLE;
    SetWindowLong(child, GWL_STYLE, style);

    // Make sure the child window can be focused
    LONG exStyle = GetWindowLong(child, GWL_EXSTYLE);
    exStyle &= ~(WS_EX_APPWINDOW | WS_EX_TOOLWINDOW);
    exStyle |= WS_EX_CONTROLPARENT;
    SetWindowLong(child, GWL_EXSTYLE, exStyle);

    // Apply the new window style and size
    SetWindowPos(child, NULL, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

    std::cout << "Window embedded: " << child << std::endl;
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    struct HandleData {
        DWORD processID;
        HWND bestHandle;
    };

    HandleData& data = *(HandleData*)lParam;
    DWORD processID = 0;
    GetWindowThreadProcessId(handle, &processID);
    if (data.processID == processID && GetWindow(handle, GW_OWNER) == 0 && IsWindowVisible(handle))
    {
        data.bestHandle = handle;
        return FALSE;
    }
    return TRUE;
}

HWND FindMainWindow(DWORD processID)
{
    struct HandleData {
        DWORD processID;
        HWND bestHandle;
    };

    HandleData data = { processID, 0 };
    EnumWindows(EnumWindowsCallback, (LPARAM)&data);
    return data.bestHandle;
}

HWND StartProcessAndGetWindow(LPCTSTR exeName)
{
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    HWND hwnd = NULL;
	
	//si.dwFlags = STARTF_USESHOWWINDOW;
    //si.wShowWindow = SW_HIDE;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWMINIMIZED;

    if (CreateProcess(NULL, const_cast<LPTSTR>(exeName), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        std::cout << "Started process: " << exeName << ", PID: " << pi.dwProcessId << std::endl;

		for (int i = 0; i<200; i++){
			
			// Try to find the main window
			hwnd = FindMainWindow(pi.dwProcessId);
			
			if (hwnd != NULL) break;
			
			
			// Wait a bit to allow the process to start and create its window
			//std::this_thread::sleep_for(std::chrono::seconds(2));
			std::this_thread::sleep_for(std::chrono::milliseconds(10));


		}
		
        if (!hwnd) {
            std::cerr << "Main window not found, enumerating all windows" << std::endl;
            // Enumerate all windows to find one belonging to the process
            HWND tempHandle = NULL;
            for (HWND window = GetTopWindow(NULL); window != NULL; window = GetNextWindow(window, GW_HWNDNEXT))
            {
                DWORD processID = 0;
                GetWindowThreadProcessId(window, &processID);
                if (processID == pi.dwProcessId && IsWindowVisible(window))
                {
                    tempHandle = window;
                    break;
                }
            }
            hwnd = tempHandle;
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        std::cerr << "Failed to start process: " << exeName << std::endl;
    }

    std::cout << "Process window handle: " << hwnd << std::endl;
    return hwnd;
}


#endif

#if defined(TEST9)

#include <windows.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include <thread> // For std::thread
#include <chrono> // For std::chrono::seconds

// Global variables
HINSTANCE hInst;
HWND hwndMSPaint;
HWND hwndNotepad;

// Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height);
HWND StartProcessAndGetWindow(LPCTSTR exeName);
BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);
HWND FindMainWindow(DWORD processID);
void StartAndEmbedProcess(LPCTSTR exeName, HWND parent, int x, int y, int width, int height);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine, int       nCmdShow)
{
    MSG msg;
    WNDCLASS wc = {0};
    HWND hwndParent;

    // Register window class
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = _T("ParentWindowClass");
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        std::cerr << "Failed to register window class" << std::endl;
        return 1;
    }

    hInst = hInstance;

    // Create the parent window
    hwndParent = CreateWindow(wc.lpszClassName, _T("Embedded Applications"),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                              NULL, NULL, hInstance, NULL);

    if (!hwndParent) {
        std::cerr << "Failed to create parent window" << std::endl;
        return 1;
    }

    ShowWindow(hwndParent, nCmdShow);
    UpdateWindow(hwndParent);

    // Start and embed mspaint asynchronously
    std::thread t1(StartAndEmbedProcess, _T("mspaint.exe"), hwndParent, 0, 0, 400, 600);
    

    // Start and embed notepad asynchronously
    std::thread t2(StartAndEmbedProcess, _T("ffplay C:/msys64/mingw64/home/demo.mkv"), hwndParent, 400, 0, 400, 600);
	t1.join(); // Ensure mspaint is embedded before starting notepad
    t2.join(); // Ensure notepad is embedded

    // Message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        {
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            int halfWidth = rcClient.right / 2;

            std::cout << "Resizing embedded windows" << std::endl;
            if (hwndMSPaint) {
                std::cout << "Resizing MSPaint window" << std::endl;
                MoveWindow(hwndMSPaint, 0, 0, halfWidth, rcClient.bottom, TRUE);
            }
            if (hwndNotepad) {
                std::cout << "Resizing Notepad window" << std::endl;
                MoveWindow(hwndNotepad, halfWidth, 0, halfWidth, rcClient.bottom, TRUE);
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height)
{
    // Remove all styles that allow the child window to be resized, moved, or decorated
    SetParent(child, parent);
    LONG style = GetWindowLong(child, GWL_STYLE);
    style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    style |= WS_CHILD | WS_VISIBLE;
    SetWindowLong(child, GWL_STYLE, style);

    // Make sure the child window can be focused
    LONG exStyle = GetWindowLong(child, GWL_EXSTYLE);
    exStyle &= ~(WS_EX_APPWINDOW | WS_EX_TOOLWINDOW);
    exStyle |= WS_EX_CONTROLPARENT;
    SetWindowLong(child, GWL_EXSTYLE, exStyle);

    // Apply the new window style and size
    SetWindowPos(child, NULL, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

    std::cout << "Window embedded: " << child << std::endl;
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    struct HandleData {
        DWORD processID;
        HWND bestHandle;
    };

    HandleData& data = *(HandleData*)lParam;
    DWORD processID = 0;
    GetWindowThreadProcessId(handle, &processID);
    if (data.processID == processID && GetWindow(handle, GW_OWNER) == 0 && IsWindowVisible(handle))
    {
        data.bestHandle = handle;
        return FALSE;
    }
    return TRUE;
}

HWND FindMainWindow(DWORD processID)
{
    struct HandleData {
        DWORD processID;
        HWND bestHandle;
    };

    HandleData data = { processID, 0 };
    EnumWindows(EnumWindowsCallback, (LPARAM)&data);
    return data.bestHandle;
}

HWND StartProcessAndGetWindow(LPCTSTR exeName)
{
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    HWND hwnd = NULL;

    // Configure STARTUPINFO to start the process minimized
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWMINIMIZED;

    if (CreateProcess(NULL, const_cast<LPTSTR>(exeName), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        std::cout << "Started process: " << exeName << ", PID: " << pi.dwProcessId << std::endl;

        // Wait a bit to allow the process to start and create its window
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Try to find the main window
        hwnd = FindMainWindow(pi.dwProcessId);
        if (!hwnd) {
            std::cerr << "Main window not found, enumerating all windows" << std::endl;
            // Enumerate all windows to find one belonging to the process
            HWND tempHandle = NULL;
            for (HWND window = GetTopWindow(NULL); window != NULL; window = GetNextWindow(window, GW_HWNDNEXT))
            {
                DWORD processID = 0;
                GetWindowThreadProcessId(window, &processID);
                if (processID == pi.dwProcessId && IsWindowVisible(window))
                {
                    tempHandle = window;
                    break;
                }
            }
            hwnd = tempHandle;
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        std::cerr << "Failed to start process: " << exeName << std::endl;
    }

    std::cout << "Process window handle: " << hwnd << std::endl;
    return hwnd;
}

void StartAndEmbedProcess(LPCTSTR exeName, HWND parent, int x, int y, int width, int height)
{
    HWND hwnd = StartProcessAndGetWindow(exeName);
    if (hwnd) {
        std::cout << "Embedding window for process: " << exeName << std::endl;
        EmbedWindow(parent, hwnd, x, y, width, height);
    } else {
        std::cerr << "Failed to get window handle for process: " << exeName << std::endl;
    }
}


#endif

#if defined(TEST10) // HUI::WindowBase2 initial
/*class WindowBase2 {
	
  public:
	WindowBase ();
	~WindowBase ();
	
	/*
		Embeds windows into the side by side layout.
	*./
	void add1 (HWND handle);
	void add2 (HWND handle);
	
	/*
		Completely shows/hides the window so it can be manipulated without letting user see.
	*./
	void show ();
	void hide ();
	
	HWND window_handle(); // get the toplevel window handle
	
	void run_blocking(); // stand-alone
	void run_nonblocking(); // should be inside some kind of loop
	std::thread run_inthread(); // in thread non blocking (or blocking if used as .run_inthread().join())
};
HWND StartProcessAndGetHandle(const char* run);	

int main(){
	auto window = WindowBase2()
	window.add1(StartProcessAndGetHandle("mspaint.exe"));
	window.add2(StartProcessAndGetHandle("ffplay C:/msys64/mingw64/home/demo.mkv"));
	window.show();
	window.run_blocking();
}
*/




#include <windows.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HWND StartProcessAndGetHandle(const char* run);
HWND FindMainWindow(DWORD processID);

class WindowBase2 {
public:
    WindowBase2();
    ~WindowBase2();
    
    void add1(HWND handle);
    void add2(HWND handle);
    
    void show();
    void hide();
    
    HWND window_handle();
    
    void run_blocking();
    void run_nonblocking();
    std::thread run_inthread();


    HWND hwndChild1;
    HWND hwndChild2;
private:
    void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height);

    HWND hwndParent;

    HINSTANCE hInst;
};

// Constructor
WindowBase2::WindowBase2() : hwndParent(NULL), hwndChild1(NULL), hwndChild2(NULL), hInst(GetModuleHandle(NULL)) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = _T("ParentWindowClass");
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        std::cerr << "Failed to register window class" << std::endl;
        exit(1);
    }

    hwndParent = CreateWindow(wc.lpszClassName, _T("Embedded Applications"),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                              NULL, NULL, hInst, this);

    if (!hwndParent) {
        std::cerr << "Failed to create parent window" << std::endl;
        exit(1);
    }
}

// Destructor
WindowBase2::~WindowBase2() {
    if (hwndParent) {
        DestroyWindow(hwndParent);
    }
}

// Embeds windows into the side by side layout
void WindowBase2::add1(HWND handle) {
    hwndChild1 = handle;
    RECT rcClient;
    GetClientRect(hwndParent, &rcClient);
    int halfWidth = rcClient.right / 2;
    EmbedWindow(hwndParent, hwndChild1, 0, 0, halfWidth, rcClient.bottom);
}

void WindowBase2::add2(HWND handle) {
    hwndChild2 = handle;
    RECT rcClient;
    GetClientRect(hwndParent, &rcClient);
    int halfWidth = rcClient.right / 2;
    EmbedWindow(hwndParent, hwndChild2, halfWidth, 0, halfWidth, rcClient.bottom);
}

// Completely shows/hides the window so it can be manipulated without letting user see
void WindowBase2::show() {
    ShowWindow(hwndParent, SW_SHOW);
    UpdateWindow(hwndParent);
}

void WindowBase2::hide() {
    ShowWindow(hwndParent, SW_HIDE);
}

// Get the top-level window handle
HWND WindowBase2::window_handle() {
    return hwndParent;
}

// Run blocking
void WindowBase2::run_blocking() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// Run non-blocking
void WindowBase2::run_nonblocking() {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// Run in thread
std::thread WindowBase2::run_inthread() {
    return std::thread([this]() { this->run_blocking(); });
}

// Embed window
void WindowBase2::EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height) {
    SetParent(child, parent);
    LONG style = GetWindowLong(child, GWL_STYLE);
    style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    style |= WS_CHILD | WS_VISIBLE;
    SetWindowLong(child, GWL_STYLE, style);

    LONG exStyle = GetWindowLong(child, GWL_EXSTYLE);
    exStyle &= ~(WS_EX_APPWINDOW | WS_EX_TOOLWINDOW);
    exStyle |= WS_EX_CONTROLPARENT;
    SetWindowLong(child, GWL_EXSTYLE, exStyle);

    SetWindowPos(child, NULL, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

    std::cout << "Window embedded: " << child << std::endl;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SIZE:
        {
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            int halfWidth = rcClient.right / 2;

            WindowBase2* pThis = (WindowBase2*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (pThis) {
                if (pThis->hwndChild1) {
                    MoveWindow(pThis->hwndChild1, 0, 0, halfWidth, rcClient.bottom, TRUE);
                }
                if (pThis->hwndChild2) {
                    MoveWindow(pThis->hwndChild2, halfWidth, 0, halfWidth, rcClient.bottom, TRUE);
                }
            }
        }
        break;

    case WM_CREATE:
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
            WindowBase2* pThis = (WindowBase2*)pcs->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

HWND StartProcessAndGetHandle(const char* run) {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    HWND hwnd = NULL;

    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWMINIMIZED;

    if (CreateProcess(NULL, const_cast<LPSTR>(run), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cout << "Started process: " << run << ", PID: " << pi.dwProcessId << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(2));

        hwnd = FindMainWindow(pi.dwProcessId);
        if (!hwnd) {
            std::cerr << "Main window not found, enumerating all windows" << std::endl;
            HWND tempHandle = NULL;
            for (HWND window = GetTopWindow(NULL); window != NULL; window = GetNextWindow(window, GW_HWNDNEXT)) {
                DWORD processID = 0;
                GetWindowThreadProcessId(window, &processID);
                if (processID == pi.dwProcessId && IsWindowVisible(window)) {
                    tempHandle = window;
                    break;
                }
            }
            hwnd = tempHandle;
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        std::cerr << "Failed to start process: " << run << std::endl;
    }

    std::cout << "Process window handle: " << hwnd << std::endl;
    return hwnd;
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam) {
    struct HandleData {
        DWORD processID;
        HWND bestHandle;
    };

    HandleData& data = *(HandleData*)lParam;
    DWORD processID = 0;
    GetWindowThreadProcessId(handle, &processID);
    if (data.processID == processID && GetWindow(handle, GW_OWNER) == 0 && IsWindowVisible(handle)) {
        data.bestHandle = handle;
        return FALSE;
    }
    return TRUE;
}

HWND FindMainWindow(DWORD processID) {
    struct HandleData {
        DWORD processID;
        HWND bestHandle;
    };

    HandleData data = { processID, 0 };
    EnumWindows(EnumWindowsCallback, (LPARAM)&data);
    return data.bestHandle;
}

int main() {
    WindowBase2 window;

    window.add1(StartProcessAndGetHandle("mspaint.exe"));
    window.add2(StartProcessAndGetHandle("notepad.exe"));

    window.show();
    window.run_blocking();
    //window.run_inthread().join();

    return 0;
}


#endif

#if defined(TEST11) // final ai

#include <windows.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

// Forward declarations
HWND StartProcessAndGetHandle(const char* run);

class WindowBase2 {
public:
    WindowBase2();
    ~WindowBase2();
    
    void add1(HWND handle);
    void add2(HWND handle);
    void add3(HWND handle);
    
    void show();
    void hide();
    
    HWND window_handle();
    
    void run_blocking();
    void run_nonblocking();
    std::thread run_inthread();

private:
    void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height);
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND hwndParent;
    HWND hwndChild1;
    HWND hwndChild2;
    HWND hwndChild3;
    HINSTANCE hInst;
};

// Constructor
WindowBase2::WindowBase2() : hwndParent(NULL), hwndChild1(NULL), hwndChild2(NULL), hInst(GetModuleHandle(NULL)) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = _T("ParentWindowClass");
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        std::cerr << "Failed to register window class" << std::endl;
        exit(1);
    }

    hwndParent = CreateWindow(wc.lpszClassName, _T("Embedded Applications"),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                              NULL, NULL, hInst, this);

    if (!hwndParent) {
        std::cerr << "Failed to create parent window" << std::endl;
        exit(1);
    }
}

// Destructor
WindowBase2::~WindowBase2() {
    if (hwndParent) {
        DestroyWindow(hwndParent);
    }
}

// Embeds windows into the side by side layout
void WindowBase2::add1(HWND handle) {
    hwndChild1 = handle;
    RECT rcClient;
    GetClientRect(hwndParent, &rcClient);
    int halfWidth = rcClient.right / 2;
    EmbedWindow(hwndParent, hwndChild1, 0, 0, halfWidth, rcClient.bottom);
}

void WindowBase2::add2(HWND handle) {
    hwndChild2 = handle;
    RECT rcClient;
    GetClientRect(hwndParent, &rcClient);
    int halfWidth = rcClient.right / 4;
    EmbedWindow(hwndParent, hwndChild2, halfWidth, 0, halfWidth, rcClient.bottom);
}
void WindowBase2::add3(HWND handle) {
    hwndChild3 = handle;
    RECT rcClient;
    GetClientRect(hwndParent, &rcClient);
    int halfWidth = rcClient.right / 4;
    EmbedWindow(hwndParent, hwndChild3, halfWidth, 0, halfWidth, rcClient.bottom);
}

// Completely shows/hides the window so it can be manipulated without letting user see
void WindowBase2::show() {
	
    ShowWindow(hwndParent, SW_SHOW);
    UpdateWindow(hwndParent);
	
}

void WindowBase2::hide() {
    ShowWindow(hwndParent, SW_HIDE);
}

// Get the top-level window handle
HWND WindowBase2::window_handle() {
    return hwndParent;
}

// Run blocking
void WindowBase2::run_blocking() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// Run non-blocking
void WindowBase2::run_nonblocking() {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// Run in thread
std::thread WindowBase2::run_inthread() { // won't ever work because "thread that creates the window owns the window" (except maybe that we created the window in the thread)
    return std::thread([this]() {
        run_blocking();
        
    });
	// TODO: <https://stackoverflow.com/questions/7489175/are-win32-windows-thread-safe> <https://learn.microsoft.com/cs-cz/windows/win32/api/winuser/nf-winuser-postmessagea?redirectedfrom=MSDN>
}

// Embed window
void WindowBase2::EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height) {
	            //ShowWindow(child, SW_SHOW);
           // UpdateWindow(child);
	  
    SetParent(child, parent);
    LONG style = GetWindowLong(child, GWL_STYLE);
    style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    style |= WS_CHILD | WS_VISIBLE;
    SetWindowLong(child, GWL_STYLE, style);

    LONG exStyle = GetWindowLong(child, GWL_EXSTYLE);
    exStyle &= ~(WS_EX_APPWINDOW | WS_EX_TOOLWINDOW);
    exStyle |= WS_EX_CONTROLPARENT;
    SetWindowLong(child, GWL_EXSTYLE, exStyle);

    SetWindowPos(child, NULL, x, y, width, height, SWP_NOZORDER /*| SWP_NOACTIVATE*/ | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	
	  
      

    std::cout << "Window embedded: " << child << std::endl;
}

LRESULT CALLBACK WindowBase2::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SIZE:
        {
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            int halfWidth = rcClient.right / 2;

            WindowBase2* pThis = (WindowBase2*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (pThis) {
                if (pThis->hwndChild1) {
                    MoveWindow(pThis->hwndChild1, 0, 0, halfWidth, rcClient.bottom, TRUE);
                }
                if (pThis->hwndChild2) {
                    MoveWindow(pThis->hwndChild2, halfWidth, 0, halfWidth, rcClient.bottom, TRUE);
                }
            }
        }
        break;

    case WM_CREATE:
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
            WindowBase2* pThis = (WindowBase2*)pcs->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

HWND StartProcessAndGetHandle(const char* run) {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    HWND hwnd = NULL;

    //si.dwFlags = STARTF_USESHOWWINDOW;
    //si.wShowWindow = SW_SHOWMINIMIZED;
	
	si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // may help: <https://stackoverflow.com/questions/49123062/unable-to-restore-windows-hidden-with-showwindow>

    if (CreateProcess(NULL, const_cast<LPSTR>(run), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cout << "Started process: " << run << ", PID: " << pi.dwProcessId << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(2));

        hwnd = 
		 [](DWORD processID)->HWND {
    HWND bestHandle = NULL;
    struct HandleData {
        DWORD processID;
        HWND bestHandle;
    } data = { processID, bestHandle };

    EnumWindows([](HWND handle, LPARAM lParam) -> BOOL {
        HandleData& data = *(HandleData*)lParam;
        DWORD processID = 0;
        GetWindowThreadProcessId(handle, &processID);
        if (data.processID == processID && GetWindow(handle, GW_OWNER) == 0 && IsWindowVisible(handle)) {
            data.bestHandle = handle;
            return FALSE;
        }
        return TRUE;
    }, (LPARAM)&data);

    return data.bestHandle;
}
		
		(pi.dwProcessId); //FindMainWindow
        if (!hwnd) {
            std::cerr << "Main window not found, enumerating all windows" << std::endl;
            HWND tempHandle = NULL;
            for (HWND window = GetTopWindow(NULL); window != NULL; window = GetNextWindow(window, GW_HWNDNEXT)) {
                DWORD processID = 0;
                GetWindowThreadProcessId(window, &processID);
                if (processID == pi.dwProcessId && IsWindowVisible(window)) {
                    tempHandle = window;
                    break;
                }
            }
            hwnd = tempHandle;
        }
		
		        if (hwnd) {
            // Show the window now that it's created
            ShowWindow(hwnd, SW_SHOW);
            UpdateWindow(hwnd);
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        std::cerr << "Failed to start process: " << run << std::endl;
    }

    std::cout << "Process window handle: " << hwnd << std::endl;
		  //ShowWindow(hwnd, SW_HIDE);
    return hwnd;
}

// The original FindMainWindow function can be directly placed into the code


int main() {
    WindowBase2 window;

    window.add1(StartProcessAndGetHandle("mspaint.exe"));
    window.add2(StartProcessAndGetHandle("notepad.exe"));
    window.add3(StartProcessAndGetHandle("ffplay C:/msys64/mingw64/home/demo.mkv"));

    window.show();
	
	for (int i = 0; i<500; i++){
		window.run_nonblocking();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	window.hide();
	
	for (int i = 0; i<500; i++){
		window.run_nonblocking();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	window.show();

    window.run_blocking();
    //window.run_inthread().join();

    return 0;
}


#endif

#if defined(TEST12) 

#include <windows.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

/*

create new window base:
 old style add()
 new embed(handle) / detach(?handle/index/....)
 show() hide() get_handle/whatever() <- no backend_window()  // TODO: handle of (new) hidden window is probably NULL 
 new struct and vector for embeded windows {handle, size_lambda, z_order, focus}
 make everythink thread safe
 
HUI::Rect

use C ABI for library as its stable

check what i can do with window from another thread

typedef/class for handle

class or function for StartProcessAndGetHandle

WindowBase2 may inherit WindowControls -> same native logic

*/

struct Rect {
	
	Rect(){
		
	}
	Rect(int16_t x, int16_t y, uint16_t w, uint16_t h){
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}
	Rect(int16_t x, int16_t y, uint16_t w, uint16_t h, Rect relative){
		this->x = x + relative.x;
		this->y = y + relative.y;
		this->w = w;
		this->h = h;
	}
	//Rect(left,top,right,bottom, relative)
	//Rect(left,top,right,bottom,w,h, relative)
	// ?rect to anchors
	
	
	int16_t x,y;
	uint16_t w,h;
};






HWND StartProcessAndGetHandle(const char* run) { // TODO: make this class with handle getter and kill destructor + proper threading
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    HWND hwnd = NULL;

    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWMINIMIZED;
	
	//si.dwFlags = STARTF_USESHOWWINDOW;
    //si.wShowWindow = SW_HIDE; // TODO: doesnt work cos the window doesnt have handle when it starts hidden; may help: <https://stackoverflow.com/questions/49123062/unable-to-restore-windows-hidden-with-showwindow>

    if (CreateProcess(NULL, const_cast<LPSTR>(run), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cout << "Started process: " << run << ", PID: " << pi.dwProcessId << std::endl;

		for (uint16_t ms = 0; ms < 2000; ms += 5) {

			std::this_thread::sleep_for(std::chrono::milliseconds(5));

			hwnd = 
			[](DWORD processID)->HWND {
				HWND bestHandle = NULL;
				struct HandleData {
					DWORD processID;
					HWND bestHandle;
				} data = { processID, bestHandle };

				EnumWindows([](HWND handle, LPARAM lParam) -> BOOL {
					HandleData& data = *(HandleData*)lParam;
					DWORD processID = 0;
					GetWindowThreadProcessId(handle, &processID);
					if (data.processID == processID && GetWindow(handle, GW_OWNER) == 0 && IsWindowVisible(handle)) {
						data.bestHandle = handle;
						return FALSE;
					}
					return TRUE;
				}, (LPARAM)&data);

				return data.bestHandle;
			
			} (pi.dwProcessId); //FindMainWindow
			
			if (hwnd) break;
		
		}
		
        if (!hwnd) {
            std::cerr << "Main window not found, enumerating all windows" << std::endl;
            HWND tempHandle = NULL;
            for (HWND window = GetTopWindow(NULL); window != NULL; window = GetNextWindow(window, GW_HWNDNEXT)) {
                DWORD processID = 0;
                GetWindowThreadProcessId(window, &processID);
                if (processID == pi.dwProcessId && IsWindowVisible(window)) {
                    tempHandle = window;
                    break;
                }
            }
            hwnd = tempHandle;
        }
		
		        if (hwnd) {
            // Show the window now that it's created
            ShowWindow(hwnd, SW_SHOW);
            UpdateWindow(hwnd);
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        std::cerr << "Failed to start process: " << run << std::endl;
    }

    std::cout << "Process window handle: " << hwnd << std::endl;
		  //ShowWindow(hwnd, SW_HIDE);
    return hwnd;
}


class WindowBase2 {
public:
    WindowBase2();
    ~WindowBase2();
	
	void embed (HWND child, Rect(*positioner)(Rect) = [](Rect rect){return rect;});
	void detach (HWND handle);
    
    /*void add1(HWND handle);
    void add2(HWND handle);
    void add3(HWND handle);*/
    
    void show();
    void hide();
    
    HWND window_handle();
    
    void run_blocking();
    void run_nonblocking();
    std::thread run_inthread();

private:
    //void EmbedWindow(HWND parent, HWND child, int x, int y, int width, int height);
    //static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND hwndParent = NULL;
	
	struct StructWindows {HWND handle; Rect(*positioner)(Rect);}; // TODO: focus (its kinda random currently) and z_order (currently defined by the order of embeding)
	std::vector<StructWindows> windows;
	
    /*HWND hwndChild1;
    HWND hwndChild2;
    HWND hwndChild3;*/
    HINSTANCE hInst;
};

// Constructor
WindowBase2::WindowBase2() : /*hwndParent(NULL), hwndChild1(NULL), hwndChild2(NULL),*/ hInst(GetModuleHandle(NULL)) {
    WNDCLASS wc = {0};
    //wc.lpfnWndProc   = WndProc;
    wc.lpfnWndProc   = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) ->LRESULT CALLBACK {
		switch (msg) {
			case WM_SIZE:
				{
					RECT rcClient;
					GetClientRect(hwnd, &rcClient);

					//int halfWidth = rcClient.right / 2;

					WindowBase2* pThis = (WindowBase2*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
					if (pThis) {
						/*if (pThis->hwndChild1) {
							MoveWindow(pThis->hwndChild1, 0, 0, halfWidth, rcClient.bottom, TRUE);
						}
						if (pThis->hwndChild2) {
							MoveWindow(pThis->hwndChild2, halfWidth, 0, halfWidth, rcClient.bottom, TRUE);
						}*/
						
						for (auto a : pThis->windows){
							std::cout<<rcClient.right<<" " <<rcClient.bottom;
							Rect rect = a.positioner(Rect(0,0,rcClient.right,rcClient.bottom));
							MoveWindow(a.handle, rect.x, rect.y, rect.w, rect.h, TRUE);
						}
					}
				}
				break;

			case WM_CREATE:
				{
					LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
					WindowBase2* pThis = (WindowBase2*)pcs->lpCreateParams;
					SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
				}
				break;

			case WM_DESTROY:
				PostQuitMessage(0);
				break;

			default:
				return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		return 0;
	};
    
	wc.hInstance     = hInst;
    wc.lpszClassName = _T("ParentWindowClass");
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        std::cerr << "Failed to register window class" << std::endl;
        exit(1);
    }

    hwndParent = CreateWindow(wc.lpszClassName, _T("Embedded Applications"),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                              NULL, NULL, hInst, this);

    if (!hwndParent) {
        std::cerr << "Failed to create parent window" << std::endl;
        exit(1);
    }
}

// Destructor
WindowBase2::~WindowBase2() {
    if (hwndParent) {
        DestroyWindow(hwndParent);
    }
}

void WindowBase2::embed (HWND child, Rect(*positioner)(Rect) ) {
	            //ShowWindow(child, SW_SHOW);
           // UpdateWindow(child);
	  
    SetParent(child, hwndParent);
    LONG style = GetWindowLong(child, GWL_STYLE);
    style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    style |= WS_CHILD | WS_VISIBLE;
    SetWindowLong(child, GWL_STYLE, style);

    LONG exStyle = GetWindowLong(child, GWL_EXSTYLE);
    exStyle &= ~(WS_EX_APPWINDOW | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);
    exStyle |= WS_EX_CONTROLPARENT;
	exStyle &= ~(WS_EX_NOACTIVATE);
    SetWindowLong(child, GWL_EXSTYLE, exStyle);

    //SetWindowPos(child, NULL, x, y, width, height, SWP_NOZORDER /*| SWP_NOACTIVATE*/ | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	this->windows.push_back({child, positioner});
	  
      

    std::cout << "Window embedded: " << child << std::endl;
}



void WindowBase2::detach (HWND handle){
	SetParent(handle, NULL);
	SetWindowLong(handle, GWL_STYLE, /* WS_THICKFRAME | */WS_CAPTION | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU | WS_THICKFRAME | WS_VISIBLE);
	SetWindowLong(handle, GWL_EXSTYLE, WS_EX_WINDOWEDGE);
}

/* // Embeds windows into the side by side layout
void WindowBase2::add1(HWND handle) {
    hwndChild1 = handle;
    RECT rcClient;
    GetClientRect(hwndParent, &rcClient);
    int halfWidth = rcClient.right / 2;
    embed(hwndChild1, 0, 0, halfWidth, rcClient.bottom);
}

void WindowBase2::add2(HWND handle) {
    hwndChild2 = handle;
    RECT rcClient;
    GetClientRect(hwndParent, &rcClient);
    int halfWidth = rcClient.right / 4;
    embed(hwndChild2, halfWidth, 0, halfWidth, rcClient.bottom);
}
void WindowBase2::add3(HWND handle) {
    hwndChild3 = handle;
    RECT rcClient;
    GetClientRect(hwndParent, &rcClient);
    int halfWidth = rcClient.right / 4;
    embed(hwndChild3, [](Rect rect){return Rect(0,0,200,200);});
}*/

// Completely shows/hides the window so it can be manipulated without letting user see
void WindowBase2::show() {
	
    ShowWindow(hwndParent, SW_SHOW);
    UpdateWindow(hwndParent);
	
}

void WindowBase2::hide() {
    ShowWindow(hwndParent, SW_HIDE);
}

// Get the top-level window handle
HWND WindowBase2::window_handle() {
    return hwndParent;
}

// Run blocking
void WindowBase2::run_blocking() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// Run non-blocking
void WindowBase2::run_nonblocking() {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// Run in thread
std::thread WindowBase2::run_inthread() { // won't ever work because "thread that creates the window owns the window" (except maybe that we created the window in the thread)
    return std::thread([this]() {
        run_blocking();
        
    });
	// TODO: <https://stackoverflow.com/questions/7489175/are-win32-windows-thread-safe> <https://learn.microsoft.com/cs-cz/windows/win32/api/winuser/nf-winuser-postmessagea?redirectedfrom=MSDN>
}

// Embed window
/*
LRESULT CALLBACK WindowBase2::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SIZE:
        {
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            int halfWidth = rcClient.right / 2;

            WindowBase2* pThis = (WindowBase2*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (pThis) {
                if (pThis->hwndChild1) {
                    MoveWindow(pThis->hwndChild1, 0, 0, halfWidth, rcClient.bottom, TRUE);
                }
                if (pThis->hwndChild2) {
                    MoveWindow(pThis->hwndChild2, halfWidth, 0, halfWidth, rcClient.bottom, TRUE);
                }
            }
        }
        break;

    case WM_CREATE:
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
            WindowBase2* pThis = (WindowBase2*)pcs->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
*/
// The original FindMainWindow function can be directly placed into the code


int main() {
    WindowBase2 window;

    window.embed(StartProcessAndGetHandle("mspaint.exe"), [](Rect rect){return Rect(0,0,rect.w/2,rect.h);});
    window.embed(StartProcessAndGetHandle("mspaint.exe"/*"notepad.exe"*/), [](Rect rect){return Rect(rect.w/2,0,rect.w/2,rect.h);});
	auto h = StartProcessAndGetHandle(/*"mspaint.exe"*/"ffplay C:/msys64/mingw64/home/demo.mkv");
    window.embed(h, [](Rect rect){return Rect(rect.w-200,rect.h-200, 200, 200);});

    window.show();
	
	for (int i = 0; i<500; i++){
		window.run_nonblocking();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	//window.hide();
	
	for (int i = 0; i<500; i++){
		window.run_nonblocking();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	window.detach (h);
	//window.show();

    window.run_blocking();
    //window.run_inthread().join();

    return 0;
}





#endif