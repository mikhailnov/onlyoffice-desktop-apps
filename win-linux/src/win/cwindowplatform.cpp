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
#include <QDesktopWidget>
#include <QGridLayout>
#include <QTimer>
#include <stdexcept>
#include <functional>
#include <QApplication>
#include <QIcon>


using namespace std::placeholders;

Q_GUI_EXPORT HICON qt_pixmapToWinHICON(const QPixmap &);

/*auto refresh_window_scaling_factor(CWindowPlatform * window) -> void
{
    QString css{AscAppManager::getWindowStylesheets(window->m_dpiRatio)};
    if ( !css.isEmpty() ) {
        window->m_pMainPanel->setStyleSheet(css);
        window->setScreenScalingFactor(window->m_dpiRatio);
    }
}*/

/*auto SetForegroundWindowInternal(HWND hWnd)
{
    AllocConsole();
    auto hWndConsole = GetConsoleWindow();
    SetWindowPos(hWndConsole, nullptr, 0, 0, 0, 0, SWP_NOACTIVATE);
    FreeConsole();
    SetForegroundWindow(hWnd);
}*/

/*auto setPlacement(HWND& hwnd, QRect& moveNormalRect, double change_factor)->void
{
    WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
    if ( GetWindowPlacement(hwnd, &wp) ) {
        if ( wp.showCmd == SW_MAXIMIZE ) {
            MONITORINFO info{sizeof(MONITORINFO)};
            GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &info);

            int dest_width_change = int(moveNormalRect.width() * (1 - change_factor));
            moveNormalRect = QRect{moveNormalRect.translated(dest_width_change/2,0).topLeft(), moveNormalRect.size() * change_factor};
            wp.rcNormalPosition.left = info.rcMonitor.left + moveNormalRect.left();
            wp.rcNormalPosition.top = info.rcMonitor.top + moveNormalRect.top();
            wp.rcNormalPosition.right = wp.rcNormalPosition.left + moveNormalRect.width();
            wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + moveNormalRect.height();
            SetWindowPlacement(hwnd, &wp);
        } else {
            QRect source_rect = QRect{QPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top),QPoint(wp.rcNormalPosition.right,wp.rcNormalPosition.bottom)};
            int dest_width_change = int(source_rect.width() * (1 - change_factor));
            QRect dest_rect = QRect{source_rect.translated(dest_width_change/2,0).topLeft(), source_rect.size() * change_factor};
            SetWindowPos(hwnd, NULL, dest_rect.left(), dest_rect.top(), dest_rect.width(), dest_rect.height(), SWP_NOZORDER);
        }
    }
}*/

CWindowPlatform::CWindowPlatform(const QRect &rect) :
    //m_winType(winType),
    m_previousState(Qt::WindowNoState),
    m_margins(QMargins()),
    m_frame(QMargins()),
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
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    m_hWnd = (HWND)winId();
    setResizeable(m_isResizeable);

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
        setContentsMargins(border, border + 1, border, border);
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

/*void CWindowPlatform::captureMouse()
{
    if (m_winType != WindowType::SINGLE) return;
    POINT cursor{0,0};
    if (GetCursorPos(&cursor)) {
        QRect _g{geometry()};
        int _window_offset_x;
        if (cursor.x - _g.x() < dpiCorrectValue(CAPTURED_WINDOW_CURSOR_OFFSET_X)) _window_offset_x = dpiCorrectValue(CAPTURED_WINDOW_CURSOR_OFFSET_X);
        else if ( cursor.x > _g.right() - dpiCorrectValue(150) ) _window_offset_x = _g.right() - dpiCorrectValue(150);
        else _window_offset_x = cursor.x - _g.x();
        move(cursor.x - _window_offset_x, cursor.y - dpiCorrectValue(CAPTURED_WINDOW_CURSOR_OFFSET_Y));
        ReleaseCapture();
        PostMessage(m_hWnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(cursor.x, cursor.y));
    }
}*/

/*void CWindowPlatform::captureMouse(int tabindex)
{
    if (m_winType != WindowType::MAIN) return;
    ReleaseCapture();
    if (tabindex >= 0 && tabindex < m_pMainPanel->tabWidget()->count()) {
        QPoint spt = m_pMainPanel->tabWidget()->tabBar()->tabRect(tabindex).topLeft() + QPoint(30, 10);
        QPoint gpt = m_pMainPanel->tabWidget()->tabBar()->mapToGlobal(spt);
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
        gpt = m_pWinPanel->mapToGlobal(gpt);
#endif
        SetCursorPos(gpt.x(), gpt.y());
        QWidget * _widget = m_pMainPanel->tabWidget()->tabBar();
        QTimer::singleShot(0,[_widget,spt] {
            INPUT _input{INPUT_MOUSE};
            _input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN;
            SendInput(1, &_input, sizeof(INPUT));
            QMouseEvent event(QEvent::MouseButtonPress, spt, Qt::LeftButton, Qt::MouseButton::NoButton, Qt::NoModifier);
            QCoreApplication::sendEvent(_widget, &event);
            _widget->grabMouse();
        });
    }
}*/

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

/*void CWindowPlatform::setScreenScalingFactor(double factor)
{
    auto normalizeTitleSize = [=](double _factor){
        QSize small_btn_size(int(TOOLBTN_WIDTH * _factor), int(TOOLBTN_HEIGHT*_factor));
        if (m_pTopButtons[BtnType::Btn_Minimize]) {
            foreach (auto btn, m_pTopButtons)
                btn->setFixedSize(small_btn_size);
        }
        m_boxTitleBtns->setFixedHeight(int(TOOLBTN_HEIGHT * _factor));
        m_boxTitleBtns->layout()->setSpacing(int(1 * _factor));
    };
    if (m_winType == WindowType::MAIN) {
        m_skipSizing = true;
        QString css(AscAppManager::getWindowStylesheets(factor));
        if ( !css.isEmpty() ) {
            double change_factor = factor / m_dpiRatio;
            m_dpiRatio = factor;
            m_pMainPanel->setStyleSheet(css);
            //m_pMainPanel->setScreenScalingFactor(factor);
            setPlacement(m_hWnd, m_moveNormalRect, change_factor);
        }
        m_skipSizing = false;
    } else
    if (m_winType == WindowType::SINGLE) {
        double change_factor = factor / m_dpiRatio;
        if (m_dpiRatio != factor) {
            if (isCustomWindowStyle()) {
                normalizeTitleSize(factor);
            }
            m_dpiRatio = factor;
        }
        if ( !WindowHelper::isWindowSystemDocked(m_hWnd) ) {
            m_skipSizing = true;
            setPlacement(m_hWnd, m_moveNormalRect, change_factor);
            m_skipSizing = false;
        }
    } else
    if (m_winType == WindowType::REPORTER) {
        QString css(AscAppManager::getWindowStylesheets(factor));
        if ( !css.isEmpty() ) {
            bool increase = factor > m_dpiRatio;
            m_dpiRatio = factor;
            m_pMainPanel->setStyleSheet(css);
            normalizeTitleSize(m_dpiRatio);
            setMinimumSize(WINDOW_MIN_WIDTH * factor, WINDOW_MIN_HEIGHT * factor);
            RECT lpWindowRect;
            GetWindowRect(m_hWnd, &lpWindowRect);
            unsigned _new_width = lpWindowRect.right - lpWindowRect.left,
                     _new_height = lpWindowRect.bottom - lpWindowRect.top;
            if ( increase )
                _new_width *= 2, _new_height *= 2;  else
                _new_width /= 2, _new_height /= 2;

            SetWindowPos(m_hWnd, NULL, 0, 0, _new_width, _new_height, SWP_NOMOVE | SWP_NOZORDER);
        }
    }
}
*/

/*void CWindowPlatform::slot_modalDialog(bool status,  WId h)
{
    Q_UNUSED(h)
    if (m_winType == WindowType::MAIN) {
        //static WindowHelper::CParentDisable * const _disabler = new WindowHelper::CParentDisable;
        std::unique_ptr<WindowHelper::CParentDisable> _disabler(new WindowHelper::CParentDisable);
        if ( status ) {
            _disabler->disable(this);
        } else _disabler->enable();
    } else
    if (m_winType == WindowType::SINGLE) {
        status ? pimpl->lockParentUI() : pimpl->unlockParentUI();
    }
}*/

/** Private **/

void CWindowPlatform::setResizeable(bool isResizeable)
{
    bool visible = isVisible();
    m_isResizeable = isResizeable;
    DWORD style = ::GetWindowLong(m_hWnd, GWL_STYLE);
    if (m_isResizeable) {
        setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
        ::SetWindowLong(m_hWnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
        ::SetWindowLong(m_hWnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX & ~WS_CAPTION);
    }
    const MARGINS shadow = {1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(m_hWnd, &shadow);
    setVisible(visible);
}

void CWindowPlatform::setResizeableAreaWidth(int width)
{
    m_resAreaWidth = (width < 0) ? 0 : width;
}

void CWindowPlatform::setContentsMargins(int left, int top, int right, int bottom)
{
    m_margins = QMargins(left, top, right, bottom);
    CWindowBase::setContentsMargins(left + m_frame.left(), top + m_frame.top(),
                                    right + m_frame.right(), bottom + m_frame.bottom());
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
        int border = int(MAIN_WINDOW_BORDER_WIDTH * m_dpiRatio);
        setContentsMargins(border, border + 1, border, border);
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
    /*case WM_ACTIVATE: {
        if (m_winType == WindowType::MAIN) {
            if (LOWORD(msg->wParam) != WA_INACTIVE) {
                WindowHelper::correctModalOrder(m_hWnd, m_modalHwnd);
                return false;
            }
        } else
        if (m_winType == WindowType::SINGLE) {
            //static bool is_mainwindow_prev;
            //is_mainwindow_prev = false;
            if (!IsWindowEnabled(m_hWnd) && m_modalHwnd && m_modalHwnd != m_hWnd) {
                if (LOWORD(msg->wParam) != WA_INACTIVE ) {
                    WindowHelper::correctModalOrder(m_hWnd, m_modalHwnd);
                    return false;
                }
            } else {
                if ( LOWORD(msg->wParam) != WA_INACTIVE ) {
                    static HWND top_window;
                    top_window = NULL;
                    EnumWindows([](HWND hw, LPARAM lp) {
                        if (!IsWindowVisible(hw) || GetWindowTextLength(hw) == 0) {
                            return TRUE;
                        }
                        if (hw == (HWND)lp) {
                            top_window = hw;
                        } else
                        if (top_window) {
                            top_window = NULL;
                            //if (AscAppManager::mainWindow() && hw == AscAppManager::mainWindow()->handle())
                                //is_mainwindow_prev = true;
                        }
                        return TRUE;
                    }, (LPARAM)m_hWnd);
                }
            }
        }
        break;
    }*/

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
            RECT frame = {0, 0, 0, 0};
            AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);
            double dpr = devicePixelRatioF();
            frame = {abs(frame.left), abs(frame.top), abs(frame.right), abs(frame.bottom)};
            m_frame = QMargins(int(double(frame.left)/dpr + 0.5),
                               int(double(frame.bottom)/dpr + 0.5),
                               int(double(frame.right)/dpr + 0.5),
                               int(double(frame.bottom)/dpr + 0.5));
            CWindowBase::setContentsMargins(m_frame + m_margins);
            m_isMaximized = true;
        } else {
            if (m_isMaximized) {
                CWindowBase::setContentsMargins(m_margins);
                m_frame = QMargins();
                m_isMaximized = false;
            }
        }
        MINMAXINFO* minMaxInfo = ( MINMAXINFO* )msg->lParam;
        if (m_minSize.required) {
            minMaxInfo->ptMinTrackSize.x = m_minSize.width;
            minMaxInfo->ptMinTrackSize.y = m_minSize.height;
        }
        if (m_maxSize.required) {
            minMaxInfo->ptMaxTrackSize.x = m_maxSize.width;
            minMaxInfo->ptMaxTrackSize.y = m_maxSize.height;
        }
        return true;
    }

    case WM_SETFOCUS: {
        if (!m_closed && IsWindowEnabled(m_hWnd)) {
            focus();
        }
        break;
    }

    /*case WM_CLOSE: {
        if (m_winType == WindowType::MAIN) {
            AscAppManager::getInstance().closeQueue().enter(sWinTag{1, size_t(this)});
        } else
        if (m_winType == WindowType::SINGLE) {
            if (m_pMainPanel) {
                QTimer::singleShot(0, m_pMainPanel, [=]() {
                    AscAppManager::getInstance().closeQueue().enter(sWinTag{2, size_t(this)});
                });
            } else return true;
        } else
        if (m_winType == WindowType::REPORTER) {
            QTimer::singleShot(0, m_pMainPanel, [=] {
                onCloseEvent();
            });
        }
        return false;
    }*/

    case WM_TIMER:
        AscAppManager::getInstance().CheckKeyboard();
        break;

    /*case WM_ENDSESSION:
        if (m_winType == WindowType::MAIN)
            CAscApplicationManagerWrapper::getInstance().CloseApplication();
        break;*/

    case UM_INSTALL_UPDATE:
        //if (m_winType == WindowType::MAIN)
            QTimer::singleShot(500, this, [=](){
                onCloseEvent();
            });
        break;

    /*case WM_ENTERSIZEMOVE: {
        if (m_winType != WindowType::REPORTER) {
            WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
            if (GetWindowPlacement(m_hWnd, &wp)) {
                MONITORINFO info{sizeof(MONITORINFO)};
                GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY), &info);
                m_moveNormalRect = QRect{QPoint{wp.rcNormalPosition.left - info.rcMonitor.left, wp.rcNormalPosition.top - info.rcMonitor.top},
                                         QSize{wp.rcNormalPosition.right - wp.rcNormalPosition.left, wp.rcNormalPosition.bottom - wp.rcNormalPosition.top}};
            }
        }
        break;
    }*/

    /*case WM_EXITSIZEMOVE: {
        if (m_winType == WindowType::MAIN) {
            setMinimumSize(0, 0);
            if (!AscAppManager::IsUseSystemScaling())
                updateScaling();
        } else
        if (m_winType == WindowType::REPORTER) {
            double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
            if (dpi_ratio != m_dpiRatio)
                setScreenScalingFactor(dpi_ratio);
        }
        break;
    }*/

    /*case WM_COPYDATA: {
        if (m_winType != WindowType::REPORTER) {
            COPYDATASTRUCT* pcds = (COPYDATASTRUCT*)msg->lParam;
            if (pcds->dwData == 1) {
                int nArgs;
                LPWSTR * szArglist = CommandLineToArgvW((WCHAR *)(pcds->lpData), &nArgs);
                if (szArglist != nullptr) {
                    std::vector<std::wstring> _v_inargs;
                    for(int i(1); i < nArgs; i++) {
                        _v_inargs.push_back(szArglist[i]);
                    }
                    if ( !_v_inargs.empty() ) {
                        AscAppManager::handleInputCmd(_v_inargs);
                    }
                }
                LocalFree(szArglist);
                if (m_winType == WindowType::MAIN) {
                    ::SetFocus(m_hWnd);
                    bringToTop();
                }
            }
        }
        break;
    }*/

    default:
        break;
    }
    return CWindowBase::nativeEvent(eventType, message, result);
}