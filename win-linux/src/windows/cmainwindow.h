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

#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H


#ifdef _WIN32
# include "win/cwindowplatform.h"
#else
# include "linux/cwindowplatform.h"
#endif
#include "cscalingwrapper.h"
#include "cmainwindowimpl.h"
#include "components/cdownloadwidget.h"
#include "components/csvgpushbutton.h"
#include <QSettings>
#include <math.h>


struct printdata;

class CMainWindow : public CWindowPlatform, public CScalingWrapper, public CMainWindowImpl
{
    Q_OBJECT
public:
    explicit CMainWindow(const QRect&);
    virtual ~CMainWindow();

    QWidget * editor(int index);
    QRect windowRect() const;
    QString documentName(int vid);
    void selectView(int id);
    void selectView(const QString& url);
    int attachEditor(QWidget *, int index = -1);
    int attachEditor(QWidget *, const QPoint&);
    int editorsCount();
    int editorsCount(const std::wstring& portal);
    bool pointInTabs(const QPoint& pt);
    bool holdView(int id) const;
    virtual void applyTheme(const std::wstring&) final;
#ifdef _UPDMODULE
    static void checkUpdates();
    static void setAutocheckUpdatesInterval(const QString&);
#endif 

private:
    void captureMouse(int);
#ifdef _UPDMODULE
    static void updateFound();
    static void updateNotFound();
    static void updateError();
#endif    
    virtual void applyWindowState(Qt::WindowState) final;
#ifdef _WIN32
    virtual void focus() final;
    virtual void onSystemDpiChanged(double) final;
#else
    virtual void dragEnterEvent(QDragEnterEvent *event) final;
    virtual void dropEvent(QDropEvent *event) final;
#endif

private slots:
#ifdef _WIN32
    virtual void slot_mainPageReady() final;
#endif
    virtual void onCloseEvent() final;

/** MainPanel **/

public:
    //WId GetHwndForKeyboard() {return ((QWidget*)m_pTabs->parent())->winId();}
    QWidget * createMainPanel(QWidget *parent, bool isCustomWindow, double scale);
    CAscTabWidget * tabWidget();
    CTabBar *tabBar();
    void goStart();
    void focusToMainPanel();
    void doOpenLocalFiles(const std::vector<std::wstring> *);
    void doOpenLocalFiles(const QStringList&);
    void doOpenLocalFiles();
    void createLocalFile(const QString& name, int format);
    void setInputFiles(QStringList *);
    void attachStartPanel(QCefView * const);
    void toggleButtonMain(bool, bool delay = false);
    bool holdUid(int) const;
    bool holdUrl(const QString&, AscEditorType) const;
    int  tabCloseRequest(int index = -1);    
#ifdef __linux
    void setMouseTracking(bool);
#endif
    void doOpenLocalFile(COpenOptions&);
    virtual void setScreenScalingFactor(double) final;
    virtual void updateScalingFactor(double) final;

public slots:
    void pushButtonMainClicked();
    void onTabClicked(int);
    void onTabChanged(int);
    void onTabCloseRequest(int);
    void onAppCloseRequest();
    void onEditorActionRequest(int, const QString&);
    void onTabsCountChanged(int, int, int);
    void onWebAppsFeatures(int id, std::wstring);
    void onCloudDocumentOpen(std::wstring, int, bool);
    void onDocumentType(int id, int type);
    void onDocumentName(void *);
    void onEditorConfig(int, std::wstring cfg);
    void onDocumentChanged(int id, bool changed);
    void onDocumentSave(int id, bool cancel = false);
    void onDocumentSaveInnerRequest(int id);
    void onDocumentDownload(void * info);
    void onDocumentLoadFinished(int);
    void onDocumentFragmented(int, bool);
    void onDocumentFragmentedBuild(int, int);
    void onFullScreen(int id, bool apply);
    void onKeyDown(void *);
    void onLocalFilesOpen(void *);
    void onLocalFileRecent(void *);
    void onLocalFileRecent(const COpenOptions&);
    void onLocalFileLocation(QString);
    void onFileLocation(int, QString);
    void onPortalOpen(QString);
    void onPortalLogout(std::wstring portal);
    void onPortalNew(QString);
    void onPortalCreate();
    void onOutsideAuth(QString);
    void onEditorAllowedClose(int);
    void onWebTitleChanged(int, std::wstring json) {}
    //virtual void onLocalOptions(const QString&);
    //void onLocalFileSaveAs(void *);
    void onDocumentPrint(void *);
    void onDocumentReady(int);

private:
    int  trySaveDocument(int);
    //virtual void refreshAboutVersion();
    //QString getSaveMessage() const;

    CTabBarWrapper*  m_pTabBarWrapper;
    CAscTabWidget *  m_pTabs;
    CSVGPushButton*  m_pButtonMain;
    QWidget*         m_pMainWidget;
    QPushButton*     m_pButtonProfile;
    CDownloadWidget* m_pWidgetDownload;
    printdata*       m_printData;
    QStringList*     m_inFiles;
    QString          m_savePortal;
    Qt::WindowState  m_mainWindowState;
    bool             m_isMaximized,
                     m_isCustomWindow;
    int              m_saveAction;

private slots:
    void slot_modalDialog(bool,  WId);
};

#endif // CMAINWINDOW_H