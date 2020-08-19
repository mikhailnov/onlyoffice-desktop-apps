
#include "cmainwindowplatform.h"
#include "utils.h"
#include "bordeless.hpp"

unique_handle g_handle;


CMainWindowPlatform::CMainWindowPlatform(const QRect& rect)
{
    QRect _window_rect{rect};
    g_handle = {borderless::create_window(&CMainWindowPlatform::WndProc, RECT{_window_rect.left(),_window_rect.top(),_window_rect.right(),_window_rect.bottom()}, this)};
    hWnd = g_handle.get();

}

auto CALLBACK CMainWindowPlatform::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept -> LRESULT
{
    if ( message == WM_NCCREATE ) {
        auto userdata = reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams;
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(userdata));
    }

    auto window = reinterpret_cast<CMainWindowPlatform *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if ( !window )
        return DefWindowProc( hWnd, message, wParam, lParam );
//static uint count=0;
//qDebug() << "main window message: " << ++count << QString(" 0x%1").arg(message,4,16,QChar('0'));

    switch ( message )
    {
    case WM_DPICHANGED:
        if ( !WindowHelper::isLeftButtonPressed() ) {
//            uint dpi_ratio = Utils::getScreenDpiRatioByHWND(int(hWnd));

//            if ( dpi_ratio != window->m_dpiRatio ) {
//                window->m_dpiRatio = dpi_ratio;
//                refresh_window_scaling_factor(window);
//                window->adjustGeometry();
//            }
        }

        break;

    case WM_NCACTIVATE:
        if (!borderless::composition_enabled()) {
            // Prevents window frame reappearing on window activation
            // in "basic" theme, where no aero shadow is present.
            return 1;
        }
        break;

    case WM_ACTIVATE: {
        if ( LOWORD(wParam) != WA_INACTIVE ) {
//            WindowHelper::correctModalOrder(hWnd, window->m_modalHwnd);
            return 0;
        }

        break;
    }

    case WM_KEYDOWN:
    {
        switch ( wParam )
        {
            case VK_F4:
                if ( HIBYTE(GetKeyState(VK_SHIFT)) & 0x80 ) {
                    qDebug() << "shift pressed";
                } else {
                    qDebug() << "shift doesn't pressed";
                }
                break;
            case VK_F5:
            {
//                window->borderlessResizeable = !window->borderlessResizeable;
                break;
            }
            case VK_F6:
            {
//                window->toggleShadow();
//                window->toggleBorderless();
//                SetFocus( winId );
                break;
            }
            case VK_F7:
            {
//                window->toggleShadow();
                break;
            }
        }

        if ( wParam != VK_TAB )
            return DefWindowProc( hWnd, message, wParam, lParam );

        SetFocus( HWND(window->m_pWinPanel->winId()) );
        break;
    }

    // ALT + SPACE or F10 system menu
    case WM_SYSCOMMAND:
    {
        if ( GET_SC_WPARAM(wParam) == SC_KEYMENU )
        {
//            RECT winrect;
//            GetWindowRect( hWnd, &winrect );
//            TrackPopupMenu( GetSystemMenu( hWnd, false ), TPM_TOPALIGN | TPM_LEFTALIGN, winrect.left + 5, winrect.top + 5, 0, hWnd, NULL);
//            break;
            return 0;
        } else
        if ( GET_SC_WPARAM(wParam) == SC_SIZE ) {
            window->setMinimumSize(MAIN_WINDOW_MIN_WIDTH * window->m_dpiRatio, MAIN_WINDOW_MIN_HEIGHT * window->m_dpiRatio);
            break;
        } else
        if ( GET_SC_WPARAM(wParam) == SC_MOVE ) {
            break;
        } else
        if (GET_SC_WPARAM(wParam) == SC_MAXIMIZE) {
            qDebug() << "wm syscommand maximized";
            break;
        }
        else
        if (GET_SC_WPARAM(wParam) == SC_RESTORE) {
//            if ( !WindowHelper::isLeftButtonPressed() )
                WindowHelper::correctWindowMinimumSize(window->handle());

            break;
        }
        else
        if (GET_SC_WPARAM(wParam) == SC_MINIMIZE) {
            break;
        }
        else
        {
            return DefWindowProc( hWnd, message, wParam, lParam );
        }
    }

    case WM_SETFOCUS:
    {
//        QString str( "Got focus" );
//        QWidget *widget = QWidget::find( ( WId )HWND( wParam ) );
//        if ( widget )
//            str += QString( " from %1 (%2)" ).arg( widget->objectName() ).arg(widget->metaObject()->className() );
//        str += "\n";
//        OutputDebugStringA( str.toLocal8Bit().data() );

        if ( IsWindowEnabled(hWnd) && window->m_pMainPanel )
            window->m_pMainPanel->focus();
        break;
    }

    case WM_NCCALCSIZE:
    {
        //this kills the window frame and title bar we added with
        //WS_THICKFRAME and WS_CAPTION
        if (window->b_borderless && wParam == TRUE) {
            auto& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
            borderless::adjust_maximized_client_rect(hWnd, params.rgrc[0]);

            if ( IsZoomed(hWnd) )
                params.rgrc[0].bottom -= 1;

            return 0;
        }
        break;
    }

    case WM_KILLFOCUS:
        break;

    case WM_CLOSE:
qDebug() << "WM_CLOSE";

        AscAppManager::getInstance().closeQueue().enter(sWinTag{1, size_t(window)});
        return 0;

    case WM_DESTROY:
    {
//        PostQuitMessage(0);
        break;
    }

    case WM_TIMER:
    {
        CAscApplicationManagerWrapper::getInstance().CheckKeyboard();
        break;
    }

//    case WM_NCPAINT:
//        return 0;

    case WM_NCHITTEST:
    {
        if ( window->b_borderless ) {
            return borderless::hit_test(hWnd, POINT{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
        }

        if ( window->borderless )
        {
            const LONG borderWidth = 8; //in pixels
            RECT winrect;
            GetWindowRect( hWnd, &winrect );
            long x = GET_X_LPARAM( lParam );
            long y = GET_Y_LPARAM( lParam );
            if ( window->borderlessResizeable )
            {
                //bottom left corner
                if ( x >= winrect.left && x < winrect.left + borderWidth &&
                    y < winrect.bottom && y >= winrect.bottom - borderWidth )
                {
                    return HTBOTTOMLEFT;
                }
                //bottom right corner
                if ( x < winrect.right && x >= winrect.right - borderWidth &&
                    y < winrect.bottom && y >= winrect.bottom - borderWidth )
                {
                    return HTBOTTOMRIGHT;
                }
                //top left corner
                if ( x >= winrect.left && x < winrect.left + borderWidth &&
                    y >= winrect.top && y < winrect.top + borderWidth )
                {
                    return HTTOPLEFT;
                }
                //top right corner
                if ( x < winrect.right && x >= winrect.right - borderWidth &&
                    y >= winrect.top && y < winrect.top + borderWidth )
                {
                    return HTTOPRIGHT;
                }
                //left border
                if ( x >= winrect.left && x < winrect.left + borderWidth )
                {
                    return HTLEFT;
                }
                //right border
                if ( x < winrect.right && x >= winrect.right - borderWidth )
                {
                    return HTRIGHT;
                }
                //bottom border
                if ( y < winrect.bottom && y >= winrect.bottom - borderWidth )
                {
                    return HTBOTTOM;
                }
                //top border
                if ( y >= winrect.top && y < winrect.top + borderWidth )
                {
                    return HTTOP;
                }
            }

            return HTCAPTION;
        }
        break;
    }

    case WM_SIZING:
        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_INTERNALPAINT);
        break;

    case WM_SIZE:
        if ( !window->skipsizing && !window->closed && window->m_pWinPanel) {
            if (wParam == SIZE_MINIMIZED) {
                window->m_pMainPanel->applyMainWindowState(Qt::WindowMinimized);
            } else {
                if ( IsWindowVisible(hWnd) ) {
                    if ( WindowHelper::isLeftButtonPressed() ) {
                        uchar dpi_ratio = Utils::getScreenDpiRatioByHWND(int(hWnd));
                        if ( dpi_ratio != window->m_dpiRatio )
                            window->setScreenScalingFactor(dpi_ratio);
                    }

                    if ( wParam == SIZE_MAXIMIZED )
                        window->m_pMainPanel->applyMainWindowState(Qt::WindowMaximized);  else
                        window->m_pMainPanel->applyMainWindowState(Qt::WindowNoState);
                }

                window->adjustGeometry();
            }
        }
        break;

    case WM_MOVING: {
#if defined(__APP_MULTI_WINDOW)
        if ( window->movedByTab() ) {
            POINT pt{0};

            if ( GetCursorPos(&pt) ) {
                AscAppManager::processMainWindowMoving(size_t(window), QPoint(pt.x, pt.y));
            }
        }

#endif
        break;
    }

    case WM_ENTERSIZEMOVE: {
        WindowHelper::correctWindowMinimumSize(window->handle());

        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        if ( GetWindowPlacement(hWnd, &wp) ) {
            MONITORINFO info{sizeof(MONITORINFO)};
            GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &info);

            window->m_moveNormalRect = QRect{QPoint{wp.rcNormalPosition.left - info.rcMonitor.left, wp.rcNormalPosition.top - info.rcMonitor.top},
                                                QSize{wp.rcNormalPosition.right - wp.rcNormalPosition.left, wp.rcNormalPosition.bottom - wp.rcNormalPosition.top}};
        }
        break;}

    case WM_EXITSIZEMOVE: {
        window->setMinimumSize(0, 0);
//#define DEBUG_SCALING
#if defined(DEBUG_SCALING) && defined(_DEBUG)
        QRect windowRect;

        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        if (GetWindowPlacement(hWnd, &wp)) {
            GET_REGISTRY_USER(reg_user)
            wp.showCmd == SW_MAXIMIZE ?
                        reg_user.setValue("maximized", true) : reg_user.remove("maximized");

            windowRect.setTopLeft(QPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top));
            windowRect.setBottomRight(QPoint(wp.rcNormalPosition.right, wp.rcNormalPosition.bottom));
            windowRect.adjust(0,0,-1,-1);
        }

        int _scr_num = QApplication::desktop()->screenNumber(windowRect.topLeft()) + 1;
        uchar dpi_ratio = _scr_num;
#else
        int dpi_ratio = Utils::getScreenDpiRatioByHWND(int(hWnd));
#endif
        if ( dpi_ratio != window->m_dpiRatio ) {
            if ( !WindowHelper::isWindowSystemDocked(hWnd) ) {
                window->setScreenScalingFactor(dpi_ratio);
            } else {
                window->m_dpiRatio = dpi_ratio;
                refresh_window_scaling_factor(window);
            }

            window->adjustGeometry();
        }

        break;
    }

    case WM_PAINT: {
#if 1
        RECT ClientRect;
        GetClientRect(hWnd, &ClientRect);

        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);

//        HBRUSH BorderBrush = CreateSolidBrush(WINDOW_BACKGROUND_COLOR);
        HBRUSH BorderBrush = CreateSolidBrush(RGB(241, 0, 0));
        FillRect(ps.hdc, &ClientRect, BorderBrush);

        DeleteObject(BorderBrush);
        EndPaint(hWnd, &ps);
#else
        RECT rect;
        GetClientRect(hWnd, &rect);

        PAINTSTRUCT ps;
        HDC hDC = ::BeginPaint(hWnd, &ps);
        HPEN hpenOld = static_cast<HPEN>(::SelectObject(hDC, ::GetStockObject(DC_PEN)));
        ::SetDCPenColor(hDC, RGB(136, 136, 136));

        HBRUSH hBrush = ::CreateSolidBrush(WINDOW_BACKGROUND_COLOR);
        HBRUSH hbrushOld = static_cast<HBRUSH>(::SelectObject(hDC, hBrush));

        ::Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);

        ::SelectObject(hDC, hbrushOld);
        ::DeleteObject(hBrush);

        ::SelectObject(hDC, hpenOld);
        ::EndPaint(hWnd, &ps);
#endif
        return 0; }

    case WM_ERASEBKGND: {
        return TRUE; }

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* minMaxInfo = ( MINMAXINFO* )lParam;
        if ( window->minimumSize.required )
        {
            minMaxInfo->ptMinTrackSize.x = window->getMinimumWidth();;
            minMaxInfo->ptMinTrackSize.y = window->getMinimumHeight();
        }

        if ( window->maximumSize.required )
        {
            minMaxInfo->ptMaxTrackSize.x = window->getMaximumWidth();
            minMaxInfo->ptMaxTrackSize.y = window->getMaximumHeight();
        }
        return 1;
    }
    case WM_ENDSESSION:
        CAscApplicationManagerWrapper::getInstance().CloseApplication();

        break;

    case WM_WINDOWPOSCHANGING: { break; }
    case WM_COPYDATA: {
        COPYDATASTRUCT* pcds = (COPYDATASTRUCT*)lParam;
        if (pcds->dwData == 1) {
            int nArgs;
            LPWSTR * szArglist = CommandLineToArgvW((WCHAR *)(pcds->lpData), &nArgs);

            if (szArglist != nullptr) {
                QStringList _in_args;
                for(int i(1); i < nArgs; i++) {
                    _in_args.append(QString::fromStdWString(szArglist[i]));
                }

                if ( _in_args.size() ) {
                    QStringList * _file_list = Utils::getInputFiles(_in_args);

                    if (_file_list->size()) {
                        window->mainPanel()->doOpenLocalFiles(*_file_list);
                    }

                    delete _file_list;
                }
            }

            ::SetFocus(hWnd);
            LocalFree(szArglist);

            window->bringToTop();
        }
        break;}
    case UM_INSTALL_UPDATE:
        window->doClose();
        break;
    default: {
        break;
    }
#if 0
    case WM_INPUTLANGCHANGE:
    case WM_INPUTLANGCHANGEREQUEST:
    {
        int _lang = LOWORD(lParam);
        m_oLanguage.Check(_lang);
    }
#endif
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void CMainWindowPlatform::set_borderless(bool enabled) {
    borderless::Style new_style = enabled ? borderless::select_borderless_style() : borderless::Style::windowed;
    borderless::Style old_style = static_cast<borderless::Style>(::GetWindowLongPtrW(g_handle.get(), GWL_STYLE));

    if (new_style != old_style) {
        b_borderless = enabled;

        ::SetWindowLongPtrW(g_handle.get(), GWL_STYLE, static_cast<LONG>(new_style));

        // when switching between borderless and windowed, restore appropriate shadow state
        borderless::set_shadow(g_handle.get(), b_borderless_shadow && (new_style != borderless::Style::windowed));

        // redraw frame
        ::SetWindowPos(g_handle.get(), nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
        ::ShowWindow(g_handle.get(), SW_SHOW);
    }
}

void CMainWindowPlatform::set_borderless_shadow(bool enabled) {
    if (b_borderless) {
        b_borderless_shadow = enabled;
        borderless::set_shadow(g_handle.get(), enabled);
    }
}

HWND CMainWindowPlatform::handle() const
{
    return  hWnd;
}
