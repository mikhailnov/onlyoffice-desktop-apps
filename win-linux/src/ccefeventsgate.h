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

#ifndef CCEFEVENTSGATE_H
#define CCEFEVENTSGATE_H

#include <QObject>
#include "ctabpanel.h"

class CCefEventsGate : public QObject
{
    Q_OBJECT

public:
    explicit CCefEventsGate(QObject *parent = nullptr);

    virtual void init(CTabPanel * const);
    CTabPanel * const panel()
    {
        return m_panel;
    }

protected:
    CTabPanel * m_panel = nullptr;

public slots:
    virtual void onPortalLogout(std::wstring portal) = 0;
    virtual void onEditorConfig(int id, std::wstring cfg) = 0;
    virtual void onEditorActionRequest(int, const QString&) = 0;
    virtual void onDocumentName(void *);
    virtual void onDocumentChanged(int id, bool changed);
    virtual void onDocumentSave(int id, bool cancel = false);
    virtual void onDocumentSaveInnerRequest(int id) = 0;
    virtual void onDocumentFragmented(int id, bool needbuild) = 0;
    virtual void onDocumentFragmentedBuild(int id, int error);
    virtual void onDocumentPrint(void *);
    virtual void onDocumentPrint(int current, uint count) = 0;
    virtual void onDocumentLoadFinished(int);
    virtual void onDocumentReady(int);
    virtual void onDocumentType(int id, int type);

    virtual void onFileLocation(int id, QString path) = 0;
    virtual void onLocalFileSaveAs(void *);

    virtual void onEditorAllowedClose(int) = 0;
    virtual void onKeyDown(void *);
    virtual void onFullScreen(int id, bool apply) = 0;

    virtual void onWebTitleChanged(int, std::wstring json) = 0;
    virtual void onWebAppsFeatures(int, std::wstring) = 0;
};

#endif // CCEFEVENTSGATE_H
