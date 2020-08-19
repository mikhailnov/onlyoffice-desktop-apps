#ifndef CMAINWINDOWPLATFORM_H
#define CMAINWINDOWPLATFORM_H

#include "cmainwindowbase.h"
#include "qwinwidget.h"
#include "cwinpanel.h"

class CMainWindowPlatform : public CMainWindowBase
{
public:
    CMainWindowPlatform(const QRect&);

    HWND handle() const;

    static auto CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept -> LRESULT;
    auto set_borderless(bool enabled) -> void;
    auto set_borderless_shadow(bool enabled) -> void;

public:
    HWND hWnd = nullptr;
    CWinPanel * m_pWinPanel = nullptr;
    bool b_borderless = false; // is the window currently borderless
    bool b_borderless_shadow = false; // should the window display a native aero shadow while borderless


};

#endif // CMAINWINDOWPLATFORM_H
