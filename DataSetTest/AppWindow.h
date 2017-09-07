#pragma once

class AppWindow : private NonCopyable
{
public:
    AppWindow() = default;
    ~AppWindow();

    void * const GetHandle() const { return window_; }

    bool Initialize(char const *name, uint32_t const width, uint32_t const height);
    void Show(bool const show);
    bool Run(std::function<bool()> const &refresh_handler);

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    HWND window_ = nullptr;
};
