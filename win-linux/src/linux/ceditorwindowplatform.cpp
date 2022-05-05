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

#include "linux/ceditorwindowplatform.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include "csplash.h"


class CEditorWindowPlatform::impl {
    CEditorWindowPlatform * m_owner = nullptr;
    WindowHelper::CParentDisable * m_disabler = nullptr;
public:
    impl(CEditorWindowPlatform * owner)
        : m_owner{owner}
        , m_disabler{new WindowHelper::CParentDisable}
    {}

    ~impl()
    {
        delete m_disabler,
        m_disabler = nullptr;
    }

    void lockParentUI(){
        m_disabler->disable(m_owner);
    }

    void unlockParentUI() {
        m_disabler->enable();
    }
};

CEditorWindowPlatform::CEditorWindowPlatform(const QRect &rect) :
    pimpl(new impl(this))
{
    if (isCustomWindowStyle())
        CX11Decoration::turnOff();
    m_dpiRatio = Utils::getScreenDpiRatio(rect.topLeft());
    setGeometry(rect);
//    setMinimumSize(WindowHelper::correctWindowMinimumSize(geometry(), {EDITOR_WINDOW_MIN_WIDTH * m_dpiRatio, MAIN_WINDOW_MIN_HEIGHT * m_dpiRatio}));
}

CEditorWindowPlatform::~CEditorWindowPlatform()
{

}

/** Public **/



/** Protected **/

void CEditorWindowPlatform::captureMouse()
{
    QMouseEvent _event(QEvent::MouseButtonRelease, QCursor::pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(AscAppManager::mainWindow(), &_event);
    setGeometry(QRect(QCursor::pos() - QPoint(300, 15), size()));
    Q_ASSERT(m_boxTitleBtns != nullptr);
    QPoint pt_in_title = (m_boxTitleBtns->geometry().topLeft() + QPoint(300,15));
    _event = {QEvent::MouseButtonPress, pt_in_title, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
//    QApplication::sendEvent(this, &_event1);
    CX11Decoration::dispatchMouseDown(&_event);
    _event = {QEvent::MouseMove, QCursor::pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
//    QApplication::sendEvent(this, &_event);
    CX11Decoration::dispatchMouseMove(&_event);
}

bool CEditorWindowPlatform::event(QEvent * event)
{
    if (event->type() == QEvent::WindowStateChange && this->isVisible()) {
        QWindowStateChangeEvent * _e_statechange = static_cast< QWindowStateChangeEvent* >( event );
        if (!isCustomWindowStyle()) return CWindowPlatform::event(event);
        if(_e_statechange->oldState() == Qt::WindowNoState && windowState() == Qt::WindowMaximized) {
            layout()->setMargin(0);
            applyWindowState(Qt::WindowMaximized);
        } else
        if (this->windowState() == Qt::WindowNoState) {
            layout()->setMargin(CX11Decoration::customWindowBorderWith() * m_dpiRatio);
            applyWindowState(Qt::WindowNoState);
        }
    } else
    if (event->type() == QEvent::Close) {
        if (!AscAppManager::mainWindow() || !AscAppManager::mainWindow()->isVisible()) {
            GET_REGISTRY_USER(reg_user);
            reg_user.setValue("position", normalGeometry());
        }
        onCloseEvent();
        event->ignore();
        return false;
    }

    return CWindowPlatform::event(event);
}

void CEditorWindowPlatform::setScreenScalingFactor(double factor)
{
    if (m_dpiRatio != factor) {
        if (isCustomWindowStyle()) {
            QSize small_btn_size(int(TOOLBTN_WIDTH * factor), int(TOOLBTN_HEIGHT*factor));
            foreach (auto btn, m_pTopButtons)
                btn->setFixedSize(small_btn_size);
            m_boxTitleBtns->setFixedHeight(int(TOOLBTN_HEIGHT * factor));
            m_boxTitleBtns->layout()->setSpacing(int(1 * factor));
        }
        m_dpiRatio = factor;
    }
}

void CEditorWindowPlatform::slot_modalDialog(bool status,  WId h)
{
    Q_UNUSED(h)
    status ? pimpl->lockParentUI() : pimpl->unlockParentUI();
}

/** Private **/

void CEditorWindowPlatform::closeEvent(QCloseEvent * e)
{
    e->ignore();
}
