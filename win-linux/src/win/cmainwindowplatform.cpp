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

#include "win/cmainwindowplatform.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include "csplash.h"
#include "clogger.h"
#include "clangater.h"
/*#include <QDesktopWidget>
#include <QGridLayout>
#include <QTimer>
#include <stdexcept>
#include <functional>
#include <QApplication>
#include <QIcon>*/

//using namespace std::placeholders;


CMainWindowPlatform::CMainWindowPlatform(const QRect &rect)
    : CWindowPlatform(rect),
    m_hWnd(nullptr),
    m_modalHwnd(nullptr),
    m_skipSizing(false),
    m_windowActivated(false)
{
    m_hWnd = (HWND)winId();
    m_window_rect = rect;

    if (m_window_rect.isEmpty())
        m_window_rect = QRect(QPoint(100, 100)*m_dpiRatio, MAIN_WINDOW_DEFAULT_SIZE * m_dpiRatio);
    QRect _screen_size = Utils::getScreenGeometry(m_window_rect.topLeft());
    if (_screen_size.intersects(m_window_rect)) {
        if (_screen_size.width() < m_window_rect.width() || _screen_size.height() < m_window_rect.height()) {
            //adjustRect(m_window_rect, _screen_size);
            m_window_rect.setLeft(_screen_size.left()),
            m_window_rect.setTop(_screen_size.top());
            if (_screen_size.width() < m_window_rect.width()) m_window_rect.setWidth(_screen_size.width());
            if (_screen_size.height() < m_window_rect.height()) m_window_rect.setHeight(_screen_size.height());
        }
    } else {
        m_window_rect = QRect(QPoint(100, 100)*m_dpiRatio, QSize(MAIN_WINDOW_MIN_WIDTH, MAIN_WINDOW_MIN_HEIGHT)*m_dpiRatio);
    }
    m_moveNormalRect = m_window_rect;
}

CMainWindowPlatform::~CMainWindowPlatform()
{

}

/** Public **/

void CMainWindowPlatform::updateScaling()
{
    double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
    if ( dpi_ratio != m_dpiRatio ) {
        if ( !WindowHelper::isWindowSystemDocked(m_hWnd) ) {
            setScreenScalingFactor(dpi_ratio);
        } else {
            m_dpiRatio = dpi_ratio;
            refresh_window_scaling_factor();
        }
        adjustGeometry();
    }
}

/** Protected **/

void CMainWindowPlatform::setScreenScalingFactor(double factor)
{
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
}

void CMainWindowPlatform::slot_modalDialog(bool status,  WId h)
{
    Q_UNUSED(h)
    //static WindowHelper::CParentDisable * const _disabler = new WindowHelper::CParentDisable;
    std::unique_ptr<WindowHelper::CParentDisable> _disabler(new WindowHelper::CParentDisable);
    if ( status ) {
        _disabler->disable(this);
    } else _disabler->enable();
}

/** Private **/

void CMainWindowPlatform::refresh_window_scaling_factor()
{
    QString css{AscAppManager::getWindowStylesheets(m_dpiRatio)};
    if ( !css.isEmpty() ) {
        m_pMainPanel->setStyleSheet(css);
        setScreenScalingFactor(m_dpiRatio);
    }
}

void CMainWindowPlatform::showEvent(QShowEvent *event)
{
    CWindowPlatform::showEvent(event);
    if (!m_windowActivated) {
        m_windowActivated = true;
        setGeometry(m_window_rect);
        CWindowPlatform::applyTheme(L"");
    }
}

void CMainWindowPlatform::changeEvent(QEvent *event)
{
    CWindowPlatform::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        if (WindowHelper::isLeftButtonPressed() && !isMinimized() && isVisible()) {
            double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
            if (dpi_ratio != m_dpiRatio)
                setScreenScalingFactor(dpi_ratio);
        }
    }
}

bool CMainWindowPlatform::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    MSG* msg = *reinterpret_cast<MSG**>(message);
#else
    MSG* msg = reinterpret_cast<MSG*>(message);
#endif

    switch (msg->message)
    {
    case WM_ACTIVATE: {
        if (LOWORD(msg->wParam) != WA_INACTIVE) {
            WindowHelper::correctModalOrder(m_hWnd, m_modalHwnd);
            return false;
        }
        break;
    }

    case WM_DPICHANGED: {
        if (!WindowHelper::isLeftButtonPressed()) {
            double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
            if (dpi_ratio != m_dpiRatio) {
                m_dpiRatio = dpi_ratio;
                refresh_window_scaling_factor();
                adjustGeometry();
            }
        } else
        if (AscAppManager::IsUseSystemScaling()) {
            updateScaling();
        }
        qDebug() << "WM_DPICHANGED: " << LOWORD(msg->wParam);
        break;
    }

    case WM_CLOSE: {
        AscAppManager::getInstance().closeQueue().enter(sWinTag{1, size_t(this)});
        return false;
    }

    case WM_ENDSESSION:
        CAscApplicationManagerWrapper::getInstance().CloseApplication();
        break;

    case UM_INSTALL_UPDATE:
        QTimer::singleShot(500, this, [=](){
            onCloseEvent();
        });
        break;

    case WM_ENTERSIZEMOVE: {
        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        if (GetWindowPlacement(m_hWnd, &wp)) {
            MONITORINFO info{sizeof(MONITORINFO)};
            GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY), &info);
            m_moveNormalRect = QRect{QPoint{wp.rcNormalPosition.left - info.rcMonitor.left, wp.rcNormalPosition.top - info.rcMonitor.top},
                                     QSize{wp.rcNormalPosition.right - wp.rcNormalPosition.left, wp.rcNormalPosition.bottom - wp.rcNormalPosition.top}};
        }
        break;
    }

    case WM_EXITSIZEMOVE: {
            setMinimumSize(0, 0);
#if defined(DEBUG_SCALING) && defined(_DEBUG)
            QRect windowRect;
            WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
            if (GetWindowPlacement(m_hWnd, &wp)) {
                GET_REGISTRY_USER(reg_user)
                wp.showCmd == SW_MAXIMIZE ?
                            reg_user.setValue("maximized", true) : reg_user.remove("maximized");

                windowRect.setTopLeft(QPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top));
                windowRect.setBottomRight(QPoint(wp.rcNormalPosition.right, wp.rcNormalPosition.bottom));
                windowRect.adjust(0,0,-1,-1);
            }

            int _scr_num = QApplication::desktop()->screenNumber(windowRect.topLeft()) + 1;
            double dpi_ratio = _scr_num;

            if ( dpi_ratio != m_dpiRatio ) {
                if ( !WindowHelper::isWindowSystemDocked(m_hWnd) ) {
                    setScreenScalingFactor(dpi_ratio);
                } else {
                    m_dpiRatio = dpi_ratio;
                    refresh_window_scaling_factor();
                }
                adjustGeometry();
            }
#else
        if (!AscAppManager::IsUseSystemScaling())
            updateScaling();
#endif
        break;
    }

    case WM_COPYDATA: {
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
            ::SetFocus(m_hWnd);
            bringToTop();
        }
        break;
    }

#if 0
    case WM_INPUTLANGCHANGE:
    case WM_INPUTLANGCHANGEREQUEST: {
        int _lang = LOWORD(msg->lParam);
        m_oLanguage.Check(_lang);
        break;
    }
#endif
    default:
        break;
    }
    return CWindowPlatform::nativeEvent(eventType, message, result);
}
