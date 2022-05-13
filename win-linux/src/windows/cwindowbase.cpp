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

#include "windows/cwindowbase.h"
#include "utils.h"
#include "ccefeventsgate.h"
#include "clangater.h"
#ifdef __linux__
# include "defines.h"
#else
# include "win/caption.h"
#endif
#include <QVariant>
#include <QSettings>
#include <QHBoxLayout>
#include <functional>
#include <QApplication>
#include <QScreen>


class CWindowBase::CWindowBasePrivate {
    bool is_custom_window_ = false;

public:
    CWindowBasePrivate() {
#ifdef Q_OS_LINUX
        GET_REGISTRY_SYSTEM(reg_system)
        GET_REGISTRY_USER(reg_user)
        if ( reg_user.value("titlebar") == "custom" ||
                reg_system.value("titlebar") == "custom" )
        {
            is_custom_window_ = true;
        }
#else
        is_custom_window_ = true;
#endif
    }

    auto is_custom_window() -> bool {
        return is_custom_window_;
    }
};


CWindowBase::CWindowBase(const QRect& r)
    : QMainWindow(nullptr)
    , m_pTopButtons(3, nullptr)
    , pimpl{new CWindowBasePrivate}
{
    if ( !r.isEmpty() )
        m_dpiRatio = Utils::getScreenDpiRatio(r.topLeft());
    else {
        QScreen * _screen = QApplication::primaryScreen();
        m_dpiRatio = Utils::getScreenDpiRatio(_screen->geometry().topLeft());
    }

    setWindowIcon(Utils::appIcon());
}

CWindowBase::CWindowBase()
    : CWindowBase(QRect())
{
}

CWindowBase::~CWindowBase()
{

}

/** Protected **/

QPushButton* CWindowBase::createToolButton(QWidget * parent, const QString& name)
{
    QPushButton * btn = new QPushButton(parent);
    btn->setObjectName(name);
    btn->setProperty("class", "normal");
    btn->setProperty("act", "tool");
    btn->setFixedSize(int(TOOLBTN_WIDTH*m_dpiRatio), int(TOOLBTN_HEIGHT*m_dpiRatio));
#ifdef __linux__
    btn->setMouseTracking(true);
#endif
    return btn;
}

QWidget* CWindowBase::createTopPanel(QWidget *parent, bool isCustom)
{
    QWidget *_boxTitleBtns;
#ifdef __linux__
    _boxTitleBtns = new QWidget(parent);
#else
    _boxTitleBtns = static_cast<QWidget*>(new Caption(parent));
#endif
    _boxTitleBtns->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout *layoutBtns = new QHBoxLayout(_boxTitleBtns);
    layoutBtns->setContentsMargins(0, 0, int(1*m_dpiRatio), 0);
    layoutBtns->setSpacing(int(1*m_dpiRatio));
    layoutBtns->addStretch();
    _boxTitleBtns->setLayout(layoutBtns);
    if (isCustom) {
        const QString names[3] = {"toolButtonMinimize", "toolButtonMaximize", "toolButtonClose"};
        std::function<void(void)> btn_methods[3] = {
            [=]{onMinimizeEvent();}, [=]{onMaximizeEvent();}, [=]{onCloseEvent();}};
        m_pTopButtons.clear();
        for (int i = 0; i < 3; i++) {
            QPushButton *btn = createToolButton(_boxTitleBtns, names[i]);
            QObject::connect(btn, &QPushButton::clicked, btn_methods[i]);
            m_pTopButtons.push_back(btn);
            layoutBtns->addWidget(btn);
        }
    }

    return _boxTitleBtns;
}

bool CWindowBase::isCustomWindowStyle()
{
    return pimpl->is_custom_window();
}

void CWindowBase::applyWindowState(Qt::WindowState s)
{
    if (m_pTopButtons[BtnType::Btn_Minimize]) {
        m_pTopButtons[BtnType::Btn_Maximize]->setProperty("class", s == Qt::WindowMaximized ? "min" : "normal") ;
        m_pTopButtons[BtnType::Btn_Maximize]->style()->polish(m_pTopButtons[BtnType::Btn_Maximize]);
    }
}

void CWindowBase::setWindowTitle(const QString& title)
{
    QMainWindow::setWindowTitle(title);
    if (m_labelTitle)
        m_labelTitle->setText(title);
}

void CWindowBase::onMinimizeEvent()
{
    QMainWindow::showMinimized();
}

void CWindowBase::onMaximizeEvent()
{
    isMaximized() ? QMainWindow::showNormal() : QMainWindow::showMaximized();
}

void CWindowBase::onCloseEvent()
{
    close();
}

void CWindowBase::focus()
{

}

void CWindowBase::applyTheme(const std::wstring&)
{
    if ( m_boxTitleBtns )
        m_boxTitleBtns->style()->polish(m_boxTitleBtns);
}
