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

#include "components/chint.h"
#include <QApplication>
#include <QTimer>
#include <QEvent>
#include <QDebug>

#define HINTPOS  QPoint(30,15)
#define HINTSIZE QSize(19,19)


CHint::CHint(QWidget *parent, const QString& text, double dpiRatio) :
    QLabel(parent, Qt::Tool | Qt::FramelessWindowHint),
    m_pParent(parent),
    m_dpiRatio(dpiRatio),
    m_activated(false)
{
    setObjectName("hintLabel");
    setAttribute(Qt::WA_DeleteOnClose);
    setText(text);
    setAlignment(Qt::AlignCenter);
    installEventFilter(this);
    parent->isEnabled() ? show() : hide();
}

CHint::~CHint()
{

}

void CHint::showEvent(QShowEvent *e)
{
    QLabel::showEvent(e);
    if (!m_activated) {
        m_activated = true;
        QFont fnt = QApplication::font();
        double k = 1.0;
#ifdef _WIN32
        k = (m_dpiRatio >= 2.0) ? 0.62 :
            (m_dpiRatio >= 1.5) ? 0.75 :
            (m_dpiRatio >= 1.25) ? 0.9 :
            (m_dpiRatio >= 1.0) ? 1.15 : 1.0;
#endif
        const int pts = int(k * m_dpiRatio * fnt.pointSize());
        setStyleSheet(QString("padding-bottom: 1px; padding-right: 1px; color: #454545; \
                               background: #ffd938; border-right: 1px solid #505050; \
                               border-bottom: 1px solid #505050; font: %1pt;")
                                .arg(pts));
        setFixedSize(m_dpiRatio * HINTSIZE);
        move(m_pParent->mapToGlobal(HINTPOS*m_dpiRatio));
    }
}

bool CHint::eventFilter(QObject *obj, QEvent *e)
{
    switch (e->type()) {
    case QEvent::MouseButtonPress:
        emit hintPressed();
        break;
    default:
        break;
    }
    return QLabel::eventFilter(obj, e);
}
