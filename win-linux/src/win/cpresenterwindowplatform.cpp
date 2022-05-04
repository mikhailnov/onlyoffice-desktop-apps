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

#include "win/cpresenterwindowplatform.h"
#include "cascapplicationmanagerwrapper.h"
//#include "defines.h"
#include "utils.h"
/*#include "csplash.h"
#include "clogger.h"
#include "clangater.h"
#include <QDesktopWidget>
#include <QGridLayout>
#include <QTimer>
#include <stdexcept>
#include <functional>
#include <QApplication>
#include <QIcon>*/

//using namespace std::placeholders;

//Q_GUI_EXPORT HICON qt_pixmapToWinHICON(const QPixmap &);

/*auto adjustRect(QRect& window_rect, const QRect& screen_size)->void
{
    window_rect.setLeft(screen_size.left()),
    window_rect.setTop(screen_size.top());
    if (screen_size.width() < window_rect.width()) window_rect.setWidth(screen_size.width());
    if (screen_size.height() < window_rect.height()) window_rect.setHeight(screen_size.height());
}*/

CPresenterWindowPlatform::CPresenterWindowPlatform(const QRect &rect) :
    m_hWnd(nullptr),
    m_windowActivated(false)
{
    m_hWnd = (HWND)winId();
    m_window_rect = rect;

    m_dpiRatio = Utils::getScreenDpiRatio(m_window_rect.topLeft());
    if (m_window_rect.isEmpty())
        m_window_rect = QRect(QPoint(100, 100)*m_dpiRatio, MAIN_WINDOW_DEFAULT_SIZE * m_dpiRatio);
    QRect _screen_size = Utils::getScreenGeometry(m_window_rect.topLeft());
    if (_screen_size.width() < m_window_rect.width() + 120 || _screen_size.height() < m_window_rect.height() + 120) {
        //adjustRect(m_window_rect, _screen_size);
        m_window_rect.setLeft(_screen_size.left()),
        m_window_rect.setTop(_screen_size.top());
        if (_screen_size.width() < m_window_rect.width()) m_window_rect.setWidth(_screen_size.width());
        if (_screen_size.height() < m_window_rect.height()) m_window_rect.setHeight(_screen_size.height());
    }
    setMinimumSize(WINDOW_MIN_WIDTH * m_dpiRatio, WINDOW_MIN_HEIGHT * m_dpiRatio);
}

CPresenterWindowPlatform::~CPresenterWindowPlatform()
{

}

/** Public **/

/*void CPresenterWindowPlatform::updateScaling()
{
    double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
    if ( dpi_ratio != m_dpiRatio ) {
        if ( !WindowHelper::isWindowSystemDocked(m_hWnd) ) {
            setScreenScalingFactor(dpi_ratio);
        } else {
            m_dpiRatio = dpi_ratio;
            //refresh_window_scaling_factor(this);
        }
        adjustGeometry();
    }
}*/

/** Protected **/

void CPresenterWindowPlatform::setScreenScalingFactor(double factor)
{
    QString css(AscAppManager::getWindowStylesheets(factor));
    if ( !css.isEmpty() ) {
        bool increase = factor > m_dpiRatio;
        m_dpiRatio = factor;
        m_pMainPanel->setStyleSheet(css);
        QSize small_btn_size(int(TOOLBTN_WIDTH * factor), int(TOOLBTN_HEIGHT*factor));
        if (m_pTopButtons[BtnType::Btn_Minimize]) {
            foreach (auto btn, m_pTopButtons)
                btn->setFixedSize(small_btn_size);
        }
        m_boxTitleBtns->setFixedHeight(int(TOOLBTN_HEIGHT * factor));
        m_boxTitleBtns->layout()->setSpacing(int(1 * factor));
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

/** Private **/

void CPresenterWindowPlatform::showEvent(QShowEvent *event)
{
    CWindowPlatform::showEvent(event);
    if (!m_windowActivated) {
        m_windowActivated = true;
        toggleBorderless(false);
        setGeometry(m_window_rect);
        CWindowPlatform::applyTheme(L"");
    }
}

bool CPresenterWindowPlatform::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    MSG* msg = *reinterpret_cast<MSG**>(message);
#else
    MSG* msg = reinterpret_cast<MSG*>(message);
#endif

    switch (msg->message)
    {
    case WM_CLOSE: {
        QTimer::singleShot(0, m_pMainPanel, [=] {
            onCloseEvent();
        });
        return false;
    }

    case WM_EXITSIZEMOVE: {
        double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
        if (dpi_ratio != m_dpiRatio)
            setScreenScalingFactor(dpi_ratio);
        break;
    }

    default:
        break;
    }
    return CWindowPlatform::nativeEvent(eventType, message, result);
}
