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

#include "linux/cpresenterwindowplatform.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include <QDesktopWidget>


/*auto adjustRect(QRect& window_rect, const double dpiRatio)->void
{
    if (window_rect.isEmpty())
        window_rect = QRect(100, 100, 1324 * dpiRatio, 800 * dpiRatio);
    QRect _screen_size = Utils::getScreenGeometry(window_rect.topLeft());
    if (_screen_size.width() < window_rect.width() + 120 ||
            _screen_size.height() < window_rect.height() + 120 )
    {
        window_rect.setLeft(_screen_size.left()),
        window_rect.setTop(_screen_size.top());
        if (_screen_size.width() < window_rect.width()) window_rect.setWidth(_screen_size.width());
        if (_screen_size.height() < window_rect.height()) window_rect.setHeight(_screen_size.height());
    }
}*/

CPresenterWindowPlatform::CPresenterWindowPlatform(const QRect &rect)
{
    GET_REGISTRY_SYSTEM(reg_system)
    GET_REGISTRY_USER(reg_user)
    if (reg_user.value("titlebar") == "custom" ||
            reg_system.value("titlebar") == "custom" )
        CX11Decoration::turnOff();
    // adjust window size
    QRect _window_rect = rect;
    m_dpiRatio = Utils::getScreenDpiRatio(QApplication::desktop()->screenNumber(rect.topLeft()));
    //adjustRect(_window_rect, m_dpiRatio);
    if (_window_rect.isEmpty())
        _window_rect = QRect(100, 100, 1324 * m_dpiRatio, 800 * m_dpiRatio);
    QRect _screen_size = Utils::getScreenGeometry(_window_rect.topLeft());
    if (_screen_size.width() < _window_rect.width() + 120 ||
            _screen_size.height() < _window_rect.height() + 120 )
    {
        _window_rect.setLeft(_screen_size.left()),
        _window_rect.setTop(_screen_size.top());
        if (_screen_size.width() < _window_rect.width()) _window_rect.setWidth(_screen_size.width());
        if (_screen_size.height() < _window_rect.height()) _window_rect.setHeight(_screen_size.height());
    }
//    setMinimumSize(WindowHelper::correctWindowMinimumSize(_window_rect, {WINDOW_MIN_WIDTH * m_dpiRatio, WINDOW_MIN_HEIGHT * m_dpiRatio}));
    setGeometry(_window_rect);
}

CPresenterWindowPlatform::~CPresenterWindowPlatform()
{

}

/** Public **/


/** Protected **/

bool CPresenterWindowPlatform::event(QEvent * event)
{
    static bool _flg_motion = false;
    static bool _flg_left_button = false;
    if (event->type() == QEvent::WindowStateChange && this->isVisible()) {
        QWindowStateChangeEvent * _e_statechange = static_cast< QWindowStateChangeEvent* >( event );
        if(_e_statechange->oldState() == Qt::WindowNoState && windowState() == Qt::WindowMaximized) {
            layout()->setMargin(0);
            applyWindowState(Qt::WindowMaximized);
        } else
        if (this->windowState() == Qt::WindowNoState) {
            layout()->setMargin(CX11Decoration::customWindowBorderWith() * m_dpiRatio);
            applyWindowState(Qt::WindowNoState);
        }
    } else
    if ( event->type() == QEvent::MouseButtonPress ) {
        _flg_left_button = static_cast<QMouseEvent *>(event)->buttons().testFlag(Qt::LeftButton);
    } else
    if ( event->type() == QEvent::MouseButtonRelease ) {
        if ( _flg_left_button && _flg_motion ) {
            double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
            if (dpi_ratio != m_dpiRatio)
                setScreenScalingFactor(dpi_ratio);
        }
        _flg_left_button = _flg_motion = false;
    } else
    if ( event->type() == QEvent::Move ) {
        if (!_flg_motion)
            _flg_motion = true;
    }

    return CWindowPlatform::event(event);
}

void CPresenterWindowPlatform::setScreenScalingFactor(double factor)
{
    QString css(AscAppManager::getWindowStylesheets(factor));
    if (!css.isEmpty()) {
        setMinimumSize(QSize(0,0));
        double change_factor = factor / m_dpiRatio;
        m_dpiRatio = factor;
        if (!isMaximized()) {
            QRect _src_rect = geometry();
            int dest_width_change = int(_src_rect.width() * (1 - change_factor));
            QRect _dest_rect = QRect{_src_rect.translated(dest_width_change/2,0).topLeft(), _src_rect.size() * change_factor};
            setGeometry(_dest_rect);
        //    setMinimumSize(WindowHelper::correctWindowMinimumSize(geometry(), {EDITOR_WINDOW_MIN_WIDTH * f, MAIN_WINDOW_MIN_HEIGHT * f}));
        };
        m_pMainPanel->setStyleSheet(css);
//        setMinimumSize(WindowHelper::correctWindowMinimumSize(_dest_rect, {WINDOW_MIN_WIDTH*factor, WINDOW_MIN_HEIGHT*factor}));
    }
}

/** Private **/

void CPresenterWindowPlatform::closeEvent(QCloseEvent * e)
{
    e->ignore();
}
