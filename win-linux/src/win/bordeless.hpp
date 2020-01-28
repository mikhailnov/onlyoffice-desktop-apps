#ifndef BORDELESS_HPP
#define BORDELESS_HPP

#include <memory>
#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")


struct hwnd_deleter {
    using pointer = HWND;
    auto operator()(HWND handle) const -> void {
        ::DestroyWindow(handle);
    }
};

using unique_handle = std::unique_ptr<HWND, hwnd_deleter>;

namespace borderless {
    enum class Style : DWORD {
        windowed         = WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX /*| WS_CLIPCHILDREN*/,
        aero_borderless  = WS_POPUP            | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CLIPCHILDREN /*| WS_CLIPSIBLINGS*/,
        basic_borderless = WS_POPUP            | WS_THICKFRAME              | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CLIPCHILDREN /*| WS_CLIPSIBLINGS*/
    };

//    auto maximized(HWND hwnd) -> bool {
//        WINDOWPLACEMENT placement;
//        if (!::GetWindowPlacement(hwnd, &placement)) {
//              return false;
//        }

//        return placement.showCmd == SW_MAXIMIZE;
//    }

    auto adjust_maximized_client_rect(HWND window, RECT& rect) -> void {
//        if (!maximized(window)) {
        if (!IsZoomed(window)) {
            return;
        }

        auto monitor = ::MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
        if (!monitor) {
            return;
        }

        MONITORINFO monitor_info{};
        monitor_info.cbSize = sizeof(monitor_info);
        if (!::GetMonitorInfoW(monitor, &monitor_info)) {
            return;
        }

        // when maximized, make the client area fill just the monitor (without task bar) rect,
        // not the whole window rect which extends beyond the monitor.
        rect = monitor_info.rcWork;
    }

    auto last_error(const std::string& message) -> std::system_error {
        return std::system_error(
            std::error_code(::GetLastError(), std::system_category()),
            message
        );
    }

    auto window_class(WNDPROC wndproc) -> const wchar_t* {
        static const wchar_t* window_class_name = [&] {
            HINSTANCE instance{GetModuleHandle(nullptr)};
//            HINSTANCE instance{nullptr};
            WNDCLASSEXW wcx{};
            wcx.cbSize = sizeof(wcx);
            wcx.style = CS_HREDRAW | CS_VREDRAW;
            wcx.hInstance = instance;
            wcx.lpfnWndProc = wndproc;
            wcx.lpszClassName = L"BorderlessWindowClass";
            wcx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
            wcx.hCursor = ::LoadCursorW(instance, IDC_ARROW);
            const ATOM result = ::RegisterClassExW(&wcx);
            if (!result) {
                throw last_error("failed to register window class");
            }
            return wcx.lpszClassName;
        }();
        return window_class_name;
    }

    auto composition_enabled() -> bool {
        BOOL composition_enabled = FALSE;
        bool success = ::DwmIsCompositionEnabled(&composition_enabled) == S_OK;
        return composition_enabled && success;
    }

    auto select_borderless_style() -> Style {
        return composition_enabled() ? Style::aero_borderless : Style::basic_borderless;
    }

    auto set_shadow(HWND handle, bool enabled) -> void {
        if (composition_enabled()) {
            static const MARGINS shadow_state[2]{ { 0,0,0,0 },{ 1,1,1,1 } };
            ::DwmExtendFrameIntoClientArea(handle, &shadow_state[enabled]);
        }
    }

    auto create_window(WNDPROC wndproc, void* userdata) -> unique_handle {
        auto handle = CreateWindowExW(
            0, window_class(wndproc), L"Borderless Window",
            static_cast<DWORD>(Style::aero_borderless), /*CW_USEDEFAULT, CW_USEDEFAULT,*/ 100, 100,
            1280, 720, nullptr, nullptr, GetModuleHandle(nullptr), userdata);
        if (!handle) {
            throw last_error("failed to create window");
        }
        return unique_handle{handle};
    }

    auto hit_test(HWND handle, POINT cursor) -> LRESULT {
        // identify borders and corners to allow resizing the window.
        // Note: On Windows 10, windows behave differently and
        // allow resizing outside the visible window frame.
        // This implementation does not replicate that behavior.
        const POINT border{
            ::GetSystemMetrics(SM_CXFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER),
            ::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER)
        };
        RECT window;
        if (!::GetWindowRect(handle, &window)) {
            return HTNOWHERE;
        }

        const auto drag = HTCAPTION;

        enum region_mask {
            client = 0b0000,
            left   = 0b0001,
            right  = 0b0010,
            top    = 0b0100,
            bottom = 0b1000,
        };

        const auto result =
            left    * (cursor.x <  (window.left   + border.x)) |
            right   * (cursor.x >= (window.right  - border.x)) |
            top     * (cursor.y <  (window.top    + border.y)) |
            bottom  * (cursor.y >= (window.bottom - border.y));

        bool borderless_resize = true;
        switch (result) {
            case left          : return borderless_resize ? HTLEFT        : drag;
            case right         : return borderless_resize ? HTRIGHT       : drag;
            case top           : return borderless_resize ? HTTOP         : drag;
            case bottom        : return borderless_resize ? HTBOTTOM      : drag;
            case top | left    : return borderless_resize ? HTTOPLEFT     : drag;
            case top | right   : return borderless_resize ? HTTOPRIGHT    : drag;
            case bottom | left : return borderless_resize ? HTBOTTOMLEFT  : drag;
            case bottom | right: return borderless_resize ? HTBOTTOMRIGHT : drag;
            case client        : return drag;
            default            : return HTNOWHERE;
        }
    }
}

#endif // BORDELESS_HPP
