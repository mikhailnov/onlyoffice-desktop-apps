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

#include "linux/cwindowplatform.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include <QDesktopWidget>
#include <QTimer>


#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
# include "cdialogopenssl.h"
#endif


CWindowPlatform::CWindowPlatform(const QRect &rect) :
    CX11Decoration(this),
    m_windowActivated(false)
{
    m_dpiRatio = Utils::getScreenDpiRatio(QApplication::desktop()->screenNumber(rect.topLeft()));
    m_window_rect = rect;
    if (m_window_rect.isEmpty())
        m_window_rect = QRect(QPoint(100, 100)*m_dpiRatio, MAIN_WINDOW_DEFAULT_SIZE * m_dpiRatio);
    QRect _screen_size = Utils::getScreenGeometry(m_window_rect.topLeft());
    if (_screen_size.intersects(m_window_rect)) {
        if (_screen_size.width() < m_window_rect.width() + 120 || _screen_size.height() < m_window_rect.height() +120) {
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

}

/** Public **/

QWidget * CWindowPlatform::handle() const
{
    return qobject_cast<QWidget *>(const_cast<CWindowPlatform *>(this));
}

void CWindowPlatform::sendSertificate(int viewid)
{
#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
    CDialogOpenSsl _dialog(this);

    NSEditorApi::CAscOpenSslData * pData = new NSEditorApi::CAscOpenSslData;
    if ( _dialog.exec() == QDialog::Accepted ) {
        _dialog.getResult(*pData);
    }

    NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_PAGE_SELECT_OPENSSL_CERTIFICATE);
    pEvent->m_pData = pData;
    AscAppManager::getInstance().GetViewById(viewid)->Apply(pEvent);
#endif
}

void CWindowPlatform::bringToTop()
{
    QMainWindow::raise();
    QMainWindow::activateWindow();
}

void CWindowPlatform::show(bool maximized)
{
    QMainWindow::show();
    if (maximized) {
        QMainWindow::setWindowState(Qt::WindowMaximized);
    }
}

void CWindowPlatform::updateScaling()
{
    double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
    if ( dpi_ratio != m_dpiRatio )
        setScreenScalingFactor(dpi_ratio);
}

void CWindowPlatform::applyTheme(const std::wstring& theme)
{
    Q_UNUSED(theme)
    QColor background = AscAppManager::themes().current().color(CTheme::ColorRole::ecrWindowBackground);
    QColor border = AscAppManager::themes().current().color(CTheme::ColorRole::ecrWindowBorder);
    setWindowColors(background, border);
}

void CWindowPlatform::setWindowColors(const QColor& background, const QColor& border)
{
    Q_UNUSED(border)
    if (!CX11Decoration::isDecorated()) {
        QPalette pal = palette();
        pal.setColor(QPalette::Window, background);
        setStyleSheet(QString("QMainWindow{border:1px solid %1;}").
                      arg(border.name()));
        setAutoFillBackground(true);
        setPalette(pal);
    }
}

/** Protected **/

bool CWindowPlatform::event(QEvent * event)
{
    static bool _flg_motion = false;
    static bool _flg_left_button = false;
    if (event->type() == QEvent::WindowStateChange && this->isVisible()) {
        QWindowStateChangeEvent * _e_statechange = static_cast< QWindowStateChangeEvent* >( event );
        CX11Decoration::setMaximized(this->windowState() == Qt::WindowMaximized ? true : false);
        layout()->setMargin(windowState() == Qt::WindowMaximized ? 0 : CX11Decoration::customWindowBorderWith() * m_dpiRatio);
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
    return CWindowBase::event(event);
}

/** Private **/

void CWindowPlatform::showEvent(QShowEvent * e)
{
    CWindowBase::showEvent(e);
    if (!m_windowActivated) {
        m_windowActivated = true;
        setGeometry(m_window_rect);
        layout()->setMargin(windowState() == Qt::WindowMaximized ? 0 : CX11Decoration::customWindowBorderWith() * m_dpiRatio);
        applyTheme(AscAppManager::themes().current().id());
    }
}

void CWindowPlatform::mouseMoveEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseMove(e);
}

void CWindowPlatform::mousePressEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseDown(e);
}

void CWindowPlatform::mouseReleaseEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseUp(e);
}

void CWindowPlatform::mouseDoubleClickEvent(QMouseEvent *)
{
    if (m_boxTitleBtns) {
        if (m_boxTitleBtns->underMouse())
            onMaximizeEvent();
    }
}
