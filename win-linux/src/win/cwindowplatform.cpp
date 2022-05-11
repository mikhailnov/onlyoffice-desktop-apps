/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#include "win/cwindowplatform.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include "csplash.h"
#include "clogger.h"
#include "clangater.h"
#include <QTimer>


CWindowPlatform::CWindowPlatform(const QRect &rect) :
    m_previousState(Qt::WindowNoState),
    m_margins(QMargins()),
    m_hWnd(nullptr),
    m_modalHwnd(nullptr),
    m_resAreaWidth(MAIN_WINDOW_BORDER_WIDTH),
    m_borderless(true),
    m_closed(false),
    m_skipSizing(false),
    m_isMaximized(false),
    m_isResizeable(true),
    m_taskBarClicked(false),
    m_windowActivated(false)
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint
                   | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint);
    m_hWnd = (HWND)winId();
    DWORD style = ::GetWindowLong(m_hWnd, GWL_STYLE);
    ::SetWindowLong(m_hWnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    const MARGINS shadow = {1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(m_hWnd, &shadow);

    m_dpiRatio = CSplash::startupDpiRatio();
    m_window_rect = rect;
    if (m_window_rect.isEmpty())
        m_window_rect = QRect(QPoint(100, 100)*m_dpiRatio, MAIN_WINDOW_DEFAULT_SIZE * m_dpiRatio);
    QRect _screen_size = Utils::getScreenGeometry(m_window_rect.topLeft());
    if (_screen_size.intersects(m_window_rect)) {
        if (_screen_size.width() < m_window_rect.width() || _screen_size.height() < m_window_rect.height()) {
            m_window_rect.setLeft(_screen_size.left()),
            m_window_rect.setTop(_screen_size.top());
            if (_screen_size.width() < m_window_rect.width()) m_window_rect.setWidth(_screen_size.width());
            if (_screen_size.height() < m_window_rect.height()) m_window_rect.setHeight(_screen_size.height());
        }
    } else {
        m_window_rect = QRect(QPoint(100, 100)*m_dpiRatio, QSize(MAIN_WINDOW_MIN_WIDTH, MAIN_WINDOW_MIN_HEIGHT)*m_dpiRatio);
    }
}

CWindowPlatform::~CWindowPlatform()
{
    m_closed = true;
}

/** Public **/

QWidget * CWindowPlatform::handle() const
{
    return qobject_cast<QWidget *>(const_cast<CWindowPlatform *>(this));
};

void CWindowPlatform::toggleBorderless(bool showmax)
{
    if (isVisible()) {
        m_borderless = !m_borderless;
        show(showmax);
    }
}

void CWindowPlatform::toggleResizeable()
{
    m_isResizeable = !m_isResizeable;
}

void CWindowPlatform::adjustGeometry()
{
    if (!isMaximized()) {
        const int border = int(MAIN_WINDOW_BORDER_WIDTH * m_dpiRatio);
        m_margins = QMargins(border, border+1, border, border);
        setContentsMargins(m_margins);
        setResizeableAreaWidth(border);
    }
}

void CWindowPlatform::bringToTop()
{
    if (IsIconic(m_hWnd)) {
        ShowWindow(m_hWnd, SW_SHOWNORMAL);
    }
    SetForegroundWindow(m_hWnd);
    SetFocus(m_hWnd);
    SetActiveWindow(m_hWnd);
}

void CWindowPlatform::setWindowColors(const QColor& background, const QColor& border)
{
    Q_UNUSED(border)
    QPalette pal = palette();
    pal.setColor(QPalette::Window, background);
    /*setStyleSheet(QString("QMainWindow{border:1px solid %1; border-top:2px solid %1;}").
                  arg(border.name()));*/
    setAutoFillBackground(true);
    setPalette(pal);
}

void CWindowPlatform::show(bool maximized)
{
    maximized ? CWindowBase::showMaximized() : CWindowBase::show();
}

void CWindowPlatform::applyTheme(const std::wstring& theme)
{
    Q_UNUSED(theme)
    QColor background = AscAppManager::themes().current().color(CTheme::ColorRole::ecrWindowBackground);
    QColor border = AscAppManager::themes().current().color(CTheme::ColorRole::ecrWindowBorder);
    setWindowColors(background, border);
}

void CWindowPlatform::updateScaling()
{
    double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
    if ( dpi_ratio != m_dpiRatio ) {
        //if ( !WindowHelper::isWindowSystemDocked(m_hWnd) ) {
            setScreenScalingFactor(dpi_ratio);
        /*} else {
            m_dpiRatio = dpi_ratio;
            refresh_window_scaling_factor(this);
        }*/
        adjustGeometry();
    }
}

/** Protected **/

void CWindowPlatform::setMinimumSize( const int width, const int height )
{
    m_minSize.required = true;
    m_minSize.width = width;
    m_minSize.height = height;
}

void CWindowPlatform::setMaximumSize( const int width, const int height )
{
    m_maxSize.required = true;
    m_maxSize.width = width;
    m_maxSize.height = height;
}

/** Private **/

void CWindowPlatform::setResizeableAreaWidth(int width)
{
    m_resAreaWidth = (width < 0) ? 0 : width;
}

int CWindowPlatform::dpiCorrectValue(int v) const
{
    return int(v * m_dpiRatio);
}

void CWindowPlatform::showEvent(QShowEvent *event)
{
    CWindowBase::showEvent(event);
    if (!m_windowActivated) {
        m_windowActivated = true;
        setGeometry(m_window_rect);
        adjustGeometry();
        applyTheme(AscAppManager::themes().current().id());
    }
}

void CWindowPlatform::changeEvent(QEvent *event)
{
    CWindowBase::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized()) {
            applyWindowState(Qt::WindowMinimized);
        } else {
            if (isVisible()) {
                /*if (WindowHelper::isLeftButtonPressed() && m_winType == WindowType::MAIN) {
                    double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
                    if (dpi_ratio != m_dpiRatio)
                        setScreenScalingFactor(dpi_ratio);
                }*/
                if (isMaximized()) {
                    applyWindowState(Qt::WindowMaximized);
                } else applyWindowState(Qt::WindowNoState);
            }
            adjustGeometry();
        }
    }
}

bool CWindowPlatform::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    MSG* msg = *reinterpret_cast<MSG**>(message);
#else
    MSG* msg = reinterpret_cast<MSG*>(message);
#endif

    switch (msg->message)
    {
    case WM_DPICHANGED: {
        double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
        onSystemDpiChanged(dpi_ratio);
        qDebug() << "WM_DPICHANGED: " << LOWORD(msg->wParam);
        break;
    }

    case WM_SYSKEYDOWN: {
        if (msg->wParam == VK_SPACE) {
            RECT winrect;
            GetWindowRect(msg->hwnd, &winrect);
            TrackPopupMenu(GetSystemMenu(msg->hwnd, false ), TPM_TOPALIGN | TPM_LEFTALIGN, winrect.left + 5, winrect.top + 5, 0, msg->hwnd, NULL);
        }
        break;
    }

    case WM_KEYDOWN: {
        if (msg->wParam == VK_F5 || msg->wParam == VK_F6 || msg->wParam == VK_F7) {
            SendMessage(msg->hwnd, WM_KEYDOWN, msg->wParam, msg->lParam);
        } else
        if (msg->wParam == VK_TAB) {
            SetFocus(HWND(winId()));
        }
        break;
    }

    case WM_SYSCOMMAND: {
        if (GET_SC_WPARAM(msg->wParam) == SC_KEYMENU) {
            return false;
        } else
        if (GET_SC_WPARAM(msg->wParam) == SC_RESTORE) {
            m_taskBarClicked = true;
        } else
        if (GET_SC_WPARAM(msg->wParam) == SC_MINIMIZE) {
            m_taskBarClicked = true;
        } else
        if (GET_SC_WPARAM(msg->wParam) == SC_SIZE) {
            break;
        } else
        if (GET_SC_WPARAM(msg->wParam) == SC_MOVE) {
            break;
        } else
        if (GET_SC_WPARAM(msg->wParam) == SC_MAXIMIZE) {
            break;
        }
        break;
    }

    case WM_NCCALCSIZE: {
        NCCALCSIZE_PARAMS& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
        if (params.rgrc[0].top != 0) params.rgrc[0].top -= 1;
        Qt::WindowStates _currentState = windowState();
        if ((m_previousState == Qt::WindowNoState && _currentState == Qt::WindowNoState)
                && !m_taskBarClicked) {
            *result = WVR_REDRAW;
        } else m_taskBarClicked = false;
        m_previousState = _currentState;
        return true;
    }

    case WM_NCHITTEST: {
        if (m_borderless) {
            *result = 0;
            const LONG border_width = (LONG)m_resAreaWidth;
            RECT winrect;
            GetWindowRect(HWND(winId()), &winrect);
            long x = GET_X_LPARAM(msg->lParam);
            long y = GET_Y_LPARAM(msg->lParam);
            if (m_isResizeable) {
                bool resizeWidth = minimumWidth() != maximumWidth();
                bool resizeHeight = minimumHeight() != maximumHeight();

                if (resizeWidth) {
                    if (x >= winrect.left && x < winrect.left + border_width) {
                        *result = HTLEFT;
                    }
                    if (x < winrect.right && x >= winrect.right - border_width) {
                        *result = HTRIGHT;
                    }
                }
                if (resizeHeight) {
                    if (y < winrect.bottom && y >= winrect.bottom - border_width) {
                        *result = HTBOTTOM;
                    }
                    if (y >= winrect.top && y < winrect.top + border_width) {
                        *result = HTTOP;
                    }
                }
                if (resizeWidth && resizeHeight) {
                    if (x >= winrect.left && x < winrect.left + border_width &&
                            y < winrect.bottom && y >= winrect.bottom - border_width) {
                        *result = HTBOTTOMLEFT;
                    }
                    if (x < winrect.right && x >= winrect.right - border_width &&
                            y < winrect.bottom && y >= winrect.bottom - border_width) {
                        *result = HTBOTTOMRIGHT;
                    }
                    if (x >= winrect.left && x < winrect.left + border_width &&
                            y >= winrect.top && y < winrect.top + border_width) {
                        *result = HTTOPLEFT;
                    }
                    if (x < winrect.right && x >= winrect.right - border_width &&
                            y >= winrect.top && y < winrect.top + border_width) {
                        *result = HTTOPRIGHT;
                    }
                }
            }
            if (*result != 0) return true;
            return false;
        }
        break;
    }

    case WM_GETMINMAXINFO: {
        if (::IsZoomed(msg->hwnd)) {
            CWindowBase::setContentsMargins(8, 9, 8, 8);
            m_isMaximized = true;
        } else
        if (m_isMaximized) {
            CWindowBase::setContentsMargins(m_margins);
            m_isMaximized = false;
        }
        return true;
    }

    case WM_SETFOCUS: {
        if (!m_closed && IsWindowEnabled(m_hWnd)) {
            focus();
        }
        break;
    }

    case WM_TIMER:
        AscAppManager::getInstance().CheckKeyboard();
        break;

    case UM_INSTALL_UPDATE:
        QTimer::singleShot(500, this, [=](){
            onCloseEvent();
        });
        break;

    default:
        break;
    }
    return CWindowBase::nativeEvent(eventType, message, result);
}
