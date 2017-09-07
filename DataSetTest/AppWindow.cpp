#include "Precomp.h"
#include "AppWindow.h"

AppWindow::~AppWindow()
{
    if (window_)
    {
        DestroyWindow(window_);
        window_ = nullptr;
    }
}

bool AppWindow::Initialize(char const *name, uint32_t const width, uint32_t const height)
{
    std::wstring class_name(strlen(name), L' ');
    std::copy(name, name + strlen(name), class_name.begin());

    WNDCLASSEX wcx{};
    wcx.cbSize = sizeof(wcx);
    wcx.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wcx.hInstance = static_cast<HINSTANCE>(GetModuleHandle(nullptr));
    wcx.lpfnWndProc = WindowProc;
    wcx.lpszClassName = class_name.c_str();

    if (INVALID_ATOM == RegisterClassEx(&wcx))
    {
        assert(false);
        return nullptr;
    }

    DWORD const style = WS_OVERLAPPEDWINDOW;
    RECT rc = { 0, 0, static_cast<int32_t>(width), static_cast<int32_t>(height) };
    AdjustWindowRect(&rc, style, FALSE);

    window_ = CreateWindow(wcx.lpszClassName, wcx.lpszClassName, style,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, wcx.hInstance, nullptr);

    if (!window_)
    {
        LOGE("Failed to create application window");
        return false;
    }

    return true;
}

void AppWindow::Show(bool const show)
{
    ShowWindow(window_, show ? SW_SHOWDEFAULT : SW_HIDE);
    UpdateWindow(window_);
}

bool AppWindow::Run(std::function<bool()> const &refresh_handler)
{
    MSG msg{};
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            if (!refresh_handler())
            {
                return false;
            }
        }
    }
    return true;
}

LRESULT CALLBACK AppWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_KEYDOWN:
        if (VK_ESCAPE == wParam)
        {
            PostQuitMessage(0);
        }
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
