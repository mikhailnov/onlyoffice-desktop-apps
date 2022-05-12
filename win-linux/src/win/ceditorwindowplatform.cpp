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

#include "win/ceditorwindowplatform.h"
#include "cascapplicationmanagerwrapper.h"
//#include "defines.h"
#include "utils.h"
#include "csplash.h"
/*#include "clogger.h"
#include "clangater.h"
#include <QDesktopWidget>
#include <QGridLayout>
#include <QTimer>
#include <stdexcept>
#include <functional>
#include <QApplication>
#include <QIcon>*/


#define CAPTURED_WINDOW_CURSOR_OFFSET_X     180
#define CAPTURED_WINDOW_CURSOR_OFFSET_Y     15

//using namespace std::placeholders;


CEditorWindowPlatform::CEditorWindowPlatform(const QRect &rect) :
    m_hWnd(nullptr),
    m_modalHwnd(nullptr),
    m_skipSizing(false),
    m_windowActivated(false)
{
    m_hWnd = (HWND)winId();
    m_window_rect = rect;
    m_dpiRatio = CSplash::startupDpiRatio();
}

CEditorWindowPlatform::~CEditorWindowPlatform()
{

}

/** Public **/

void CEditorWindowPlatform::updateScaling()
{
    double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
    if ( dpi_ratio != m_dpiRatio ) {
        if ( !WindowHelper::isWindowSystemDocked(m_hWnd) ) {
            setScreenScalingFactor(dpi_ratio);
        } else {
            m_dpiRatio = dpi_ratio;
            setScreenScalingFactor(dpi_ratio);
            //refresh_window_scaling_factor(this);
        }
        adjustGeometry();
    }
}

/** Protected **/

void CEditorWindowPlatform::captureMouse()
{
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
}

void CEditorWindowPlatform::setScreenScalingFactor(double factor)
{
    double change_factor = factor / m_dpiRatio;
    if (m_dpiRatio != factor) {
        if (isCustomWindowStyle()) {
            QSize small_btn_size(int(TOOLBTN_WIDTH * factor), int(TOOLBTN_HEIGHT*factor));
            if (m_pTopButtons[BtnType::Btn_Minimize]) {
                foreach (auto btn, m_pTopButtons)
                    btn->setFixedSize(small_btn_size);
            }
            m_boxTitleBtns->setFixedHeight(int(TOOLBTN_HEIGHT * factor));
            m_boxTitleBtns->layout()->setSpacing(int(1 * factor));
        }
        m_dpiRatio = factor;
    }
    if ( !WindowHelper::isWindowSystemDocked(m_hWnd) ) {
        m_skipSizing = true;
        setPlacement(m_hWnd, m_moveNormalRect, change_factor);
        m_skipSizing = false;
    }
}

void CEditorWindowPlatform::slot_modalDialog(bool status,  WId h)
{
    Q_UNUSED(h)
    /*if (m_winType == WindowType::SINGLE) {
        status ? pimpl->lockParentUI() : pimpl->unlockParentUI();
    }*/
}

/** Private **/

int CEditorWindowPlatform::dpiCorrectValue(int v) const
{
    return int(v * m_dpiRatio);
}

void CEditorWindowPlatform::showEvent(QShowEvent *event)
{
    CWindowPlatform::showEvent(event);
    if (!m_windowActivated) {
        m_windowActivated = true;
        setGeometry(m_window_rect);
    }
}

bool CEditorWindowPlatform::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    MSG* msg = *reinterpret_cast<MSG**>(message);
#else
    MSG* msg = reinterpret_cast<MSG*>(message);
#endif

    switch (msg->message)
    {
    case WM_ACTIVATE: {
        if (!IsWindowEnabled(m_hWnd) && m_modalHwnd && m_modalHwnd != m_hWnd) {
            if (LOWORD(msg->wParam) != WA_INACTIVE ) {
                SetActiveWindow(m_modalHwnd);
                SetWindowPos(m_hWnd, m_modalHwnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
                return 0;
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
                    }
                    return TRUE;
                }, (LPARAM)m_hWnd);
            }
        }
        break;
    }

    case WM_DPICHANGED: {
        if (!WindowHelper::isLeftButtonPressed() || AscAppManager::IsUseSystemScaling()) {
            double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
            if (dpi_ratio != m_dpiRatio) {
                setScreenScalingFactor(dpi_ratio);
            }
        }
        qDebug() << "WM_DPICHANGED: " << LOWORD(msg->wParam);
        break;
    }

    case WM_CLOSE: {
        if (m_pMainPanel) {
            QTimer::singleShot(0, m_pMainPanel, [=]() {
                AscAppManager::getInstance().closeQueue().enter(sWinTag{2, size_t(this)});
            });
        } else return true;
    }

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
        }
        break;
    }

    default:
        break;
    }
    return CWindowPlatform::nativeEvent(eventType, message, result);
}
