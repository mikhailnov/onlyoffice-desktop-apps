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
//#include <QDesktopWidget>
#include <QTimer>


#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
# include "cdialogopenssl.h"
#endif


CWindowPlatform::CWindowPlatform() :
    CX11Decoration(this),
    m_windowActivated(false)
{
    if ( isCustomWindowStyle() ) {
        CX11Decoration::turnOff();

#ifdef __linux__
    m_isCustomWindow = !CX11Decoration::isDecorated();
#endif
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
    //if (m_winType == WindowType::MAIN) {
        if ( !CX11Decoration::isDecorated() ) {
            QPalette _palette(palette());
            _palette.setColor(QPalette::Background, AscAppManager::themes()
                          .current().color(CTheme::ColorRole::ecrWindowBackground));
            setStyleSheet(QString("QMainWindow{border:1px solid %1;}")
                          .arg(QString::fromStdWString(AscAppManager::themes().current()
                          .value(CTheme::ColorRole::ecrWindowBorder))));
            setAutoFillBackground(true);
            setPalette(_palette);
        }
    //}
}

/** Protected **/

/*void CWindowPlatform::captureMouse(int tabindex)
{
    if (m_winType != WindowType::MAIN) return;
    if (tabindex >= 0 && tabindex < tabWidget()->count()) {
        QPoint spt = tabWidget()->tabBar()->tabRect(tabindex).topLeft() + QPoint(30, 10);
        QTimer::singleShot(0, this, [=] {
            QMouseEvent event(QEvent::MouseButtonPress, spt, Qt::LeftButton, Qt::MouseButton::NoButton, Qt::NoModifier);
            QCoreApplication::sendEvent((QWidget *)tabWidget()->tabBar(), &event);
            tabWidget()->tabBar()->grabMouse();
        });
    }
}*/

bool CWindowPlatform::event(QEvent * event)
{
    if (event->type() == QEvent::WindowStateChange && this->isVisible()) {
        CX11Decoration::setMaximized(this->windowState() == Qt::WindowMaximized ? true : false);
    }
    return CWindowBase::event(event);
}

/** Private **/

/*void CWindowPlatform::closeEvent(QCloseEvent * e)
{
    if (m_winType == WindowType::MAIN)
        onCloseEvent();
    e->ignore();
}*/

void CWindowPlatform::showEvent(QShowEvent * e)
{
    QMainWindow::showEvent(e);
    if (!m_windowActivated) {
        m_windowActivated = true;
        if (!CX11Decoration::isDecorated()) {
            QPalette _palette(palette());
            _palette.setColor(QPalette::Background, AscAppManager::themes().current()
                              .color(CTheme::ColorRole::ecrWindowBackground));
            setStyleSheet(QString("QMainWindow{border:1px solid %1;}")
                          .arg(QString::fromStdWString(AscAppManager::themes().current().
                                                       value(CTheme::ColorRole::ecrWindowBorder))));
            setAutoFillBackground(true);
            setPalette(_palette);
        }
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

/*void CWindowPlatform::dragEnterEvent(QDragEnterEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.length() != 1)
        return;

    QSet<QString> _exts;
    _exts << "docx" << "doc" << "odt" << "rtf" << "txt" << "doct" << "dotx" << "ott";
    _exts << "html" << "mht" << "epub";
    _exts << "pptx" << "ppt" << "odp" << "ppsx" << "pptt" << "potx" << "otp";
    _exts << "xlsx" << "xls" << "ods" << "csv" << "xlst" << "xltx" << "ots";
    _exts << "pdf" << "djvu" << "xps";
    _exts << "plugin";

    QFileInfo oInfo(urls[0].toString());

    if (_exts.contains(oInfo.suffix()))
        event->acceptProposedAction();
}

void CWindowPlatform::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.length() != 1)
        return;

    QString _path = urls[0].path();

    Utils::keepLastPath(LOCAL_PATH_OPEN, _path);
    COpenOptions opts = {"", etLocalFile, _path};
    opts.wurl = _path.toStdWString();

    std::wstring::size_type nPosPluginExt = opts.wurl.rfind(L".plugin");
    std::wstring::size_type nUrlLen = opts.wurl.length();
    if ((nPosPluginExt != std::wstring::npos) && ((nPosPluginExt + 7) == nUrlLen))
    {
        // register plugin
        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
        pEvent->m_nType = ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_ADD_PLUGIN;
        NSEditorApi::CAscAddPlugin* pData = new NSEditorApi::CAscAddPlugin();
        pData->put_Path(opts.wurl);
        pEvent->m_pData = pData;

        AscAppManager::getInstance().Apply(pEvent);
    }
    else
    {
        ((CMainPanel *)m_pMainPanel)->doOpenLocalFile(opts);
    }
    event->acceptProposedAction();
}*/

/*void CWindowPlatform::slot_windowChangeState(Qt::WindowState s)
{
    if (s == Qt::WindowFullScreen) {
        GET_REGISTRY_USER(reg_user)
        reg_user.setValue("position", normalGeometry());
        reg_user.setValue("maximized", windowState().testFlag(Qt::WindowMaximized));
//        reg_user.setValue("windowstate", saveState());
//        showFullScreen();
    } else {
        if ( s == Qt::WindowMinimized && windowState().testFlag(Qt::WindowMaximized) ) {
            CX11Decoration::setMinimized();
        } else setWindowState(s);

        switch (s) {
        case Qt::WindowMaximized:
        case Qt::WindowMinimized:
            break;
        default:
        case Qt::WindowNoState:
            activateWindow();
            break;
        }
    }
}*/

/*void CWindowPlatform::onSizeEvent(int type)
{
    //CWindowBase::onSizeEvent(type);
    if ( type == Qt::WindowMinimized ) {
//        m_pTopButtons[WindowHelper::Btn_Maximize]->setProperty("class", s == Qt::WindowMaximized ? "min" : "normal") ;
//        m_pTopButtons[WindowHelper::Btn_Maximize]->style()->polish(m_pTopButtons[WindowHelper::Btn_Maximize]);
    }
}*/

/*void CWindowPlatform::onCloseEvent()
{
    QWidget * mainView = m_pMainPanel->findChild<QWidget *>("mainView");
    if ( mainView ) {
        mainView->setObjectName("destroyed");
        AscAppManager::getInstance().DestroyCefView(
                ((QCefView *)mainView)->GetCefView()->GetId() );
    }
    hide();
}*/

/*void CWindowPlatform::bringToTop() const
{
    QApplication::setActiveWindow(const_cast<CWindowPlatform *>(this));
}*/
