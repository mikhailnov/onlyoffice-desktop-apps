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

#include "linux/cmainwindowplatform.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include <QDesktopWidget>


CMainWindowPlatform::CMainWindowPlatform(const QRect &rect)
{
    setAcceptDrops(true);
    setObjectName("MainWindow");

    // adjust window size
    QRect _window_rect = rect;
    m_dpiRatio = Utils::getScreenDpiRatio( QApplication::desktop()->screenNumber(_window_rect.topLeft()) );
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
    setGeometry(_window_rect);
}

CMainWindowPlatform::~CMainWindowPlatform()
{

}

/** Public **/


/** Protected **/

bool CMainWindowPlatform::event(QEvent * event)
{
    static bool _flg_motion = false;
    static bool _flg_left_button = false;
    if (event->type() == QEvent::WindowStateChange && this->isVisible()) {
        QWindowStateChangeEvent * _e_statechange = static_cast< QWindowStateChangeEvent* >( event );
        if( _e_statechange->oldState() == Qt::WindowNoState && windowState() == Qt::WindowMaximized ) {
            applyWindowState(Qt::WindowMaximized);
        } else
        if (this->windowState() == Qt::WindowNoState) {
            applyWindowState(Qt::WindowNoState);
        } else
        if (this->windowState() == Qt::WindowMinimized) {
            applyWindowState(Qt::WindowMinimized);
        }
    } else
    if ( event->type() == QEvent::MouseButtonPress ) {
        _flg_left_button = static_cast<QMouseEvent *>(event)->buttons().testFlag(Qt::LeftButton);
    } else
    if ( event->type() == QEvent::MouseButtonRelease ) {
        if ( _flg_left_button && _flg_motion ) {
            updateScaling();
        }
        _flg_left_button = _flg_motion = false;
    } else
    if ( event->type() == QEvent::Move ) {
        if (!_flg_motion)
            _flg_motion = true;
    }

    return CWindowPlatform::event(event);
}

void CMainWindowPlatform::setScreenScalingFactor(double factor)
{
    CX11Decoration::onDpiChanged(factor);
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
        //m_pMainPanel->setScreenScalingFactor(factor);
        // TODO: skip window min size for usability test
//        setMinimumSize(WindowHelper::correctWindowMinimumSize(_src_rect, {MAIN_WINDOW_MIN_WIDTH * factor, MAIN_WINDOW_MIN_HEIGHT * factor}));
    }
}

void CMainWindowPlatform::slot_modalDialog(bool status,  WId h)
{
    Q_UNUSED(h)
    std::unique_ptr<WindowHelper::CParentDisable> _disabler(new WindowHelper::CParentDisable);
    if ( status ) {
        _disabler->disable(this);
    } else _disabler->enable();
}

/** Private **/

void CMainWindowPlatform::closeEvent(QCloseEvent * e)
{
    onCloseEvent();
    e->ignore();
}
