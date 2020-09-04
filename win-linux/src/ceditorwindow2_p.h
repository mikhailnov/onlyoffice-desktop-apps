#ifndef CEDITORWINDOW2_P_H
#define CEDITORWINDOW2_P_H

#include "ccefeventsgate.h"
#include "defines.h"
#include "ceditorwindow2.h"
#include "cellipsislabel.h"

#ifdef Q_OS_WIN
# include "win/cwindowplatform_p.h"
#else
#endif

namespace {
const QString g_css =
        "#mainPanel{background-color:%1;}"
        "#box-title-tools{background-color:%1;}"
        "QPushButton[act=tool]:hover{background-color:rgba(0,0,0,20%)}"
        "QPushButton#toolButtonClose:hover{background-color:#d42b2b;}"
        "QPushButton#toolButtonClose:pressed{background-color:#d75050;}"
        "#labelTitle{color:#444;font-size:11px;}"
        "#iconuser{color:#fff;font-size:11px;}"
        "#mainPanel[window=pretty] QPushButton[act=tool]:hover{background-color:rgba(255,255,255,20%)}"
        "#mainPanel[window=pretty] QPushButton#toolButtonMinimize,"
        "#mainPanel[window=pretty] QPushButton#toolButtonClose {background-image:url(:/minclose_light.png);}"
        "#mainPanel[window=pretty] QPushButton#toolButtonClose:hover{background-color:#d42b2b;}"
        "#mainPanel[window=pretty] QPushButton#toolButtonMaximize{background-image:url(:/max_light.png);}"
        "#mainPanel[window=pretty] #labelTitle{color:#fff;}"
        "#mainPanel[zoom=\"2x\"] #toolButtonMinimize,#mainPanel[zoom=\"2x\"] #toolButtonClose,"
        "#mainPanel[zoom=\"2x\"] #toolButtonMaximize{padding: 10px 24px 14px;}"
        "#mainPanel[zoom=\"2x\"] #iconuser,"
        "#mainPanel[zoom=\"2x\"] #labelTitle{font-size:24px;}"
        "#mainPanel[zoom=\"2x\"][window=pretty] QPushButton#toolButtonMinimize,"
        "#mainPanel[zoom=\"2x\"][window=pretty] QPushButton#toolButtonClose {background-image:url(:/minclose_light_2x.png);}"
        "#mainPanel[zoom=\"2x\"][window=pretty] QPushButton#toolButtonMaximize{background-image:url(:/max_light_2x.png);}";

auto prepare_editor_css(int type) -> QString {
    switch (type) {
    default: return g_css.arg(WINDOW_BACKGROUND_COLOR);
    case etDocument: return g_css.arg(TAB_COLOR_DOCUMENT);
    case etPresentation: return g_css.arg(TAB_COLOR_PRESENTATION);
    case etSpreadsheet: return g_css.arg(TAB_COLOR_SPREADSHEET);
    }
}
}

class CEditorEventsReceiver: public CCefEventsGate
{
public:
    explicit CEditorEventsReceiver()
        : CCefEventsGate()
    {}

    void onPortalLogout(std::wstring portal) override {}
    void onEditorConfig(int id, std::wstring cfg) override {}
    void onDocumentName(void *) override {}
    void onDocumentChanged(int id, bool changed) override {}
    void onDocumentSave(int id, bool cancel = false) override {}
    void onDocumentSaveInnerRequest(int id) override {}
    void onDocumentFragmented(int id, bool needbuild) override {}
    void onDocumentFragmentedBuild(int id, int error) override {}
    void onDocumentPrint(void *) override {}
    void onDocumentPrint(int current, uint count) override {}
    void onDocumentLoadFinished(int) override {}
    void onDocumentReady(int) override {}
    void onDocumentType(int, int type) override {}

    void onFileLocation(int id, QString path) override {}
    void onLocalFileSaveAs(void *) override {}

    void onEditorAllowedClose(int) override {}
    void onKeyDown(void *) override {}
    void onFullScreen(int id, bool apply) override {}

    void onWebTitleChanged(int, std::wstring json) override {}
    void onWebAppsFeatures(int, std::wstring) override {}
};

class CEditorWindow2Private : public CWindowPlatformPrivate
{
    friend class CEditorEventsReceiver;

public:
    CEditorEventsReceiver m_listener;
    CTabPanel * m_editor_panel = nullptr;
    QString m_css;
    QWidget * m_pMainPanel = nullptr;
    QWidget * m_boxTitleBtns = nullptr;
    QWidget * m_pMainView = nullptr;
    CEllipsisLabel * iconuser = nullptr;

    QPushButton * m_buttonMinimize = nullptr;
    QPushButton * m_buttonMaximize = nullptr;
    QPushButton * m_buttonClose = nullptr;
    CEllipsisLabel * m_labelTitle = nullptr;

    auto init(CEditorWindow2 * owner, CTabPanel * panel) {
        CWindowPlatformPrivate::init(owner);
        m_editor_panel = panel;

        m_css = {prepare_editor_css(extendableTitle() ? panel->data()->contentType() : etUndefined)};
    }

    auto extendableTitle() -> bool {
        return true;
    }

    auto panel() {
        return m_editor_panel;
    }

    auto iconUser() {
        if ( !iconuser ) {
            CEditorWindow2 * window = reinterpret_cast<CEditorWindow2 *>(q_ptr);

            iconuser = new CEllipsisLabel(m_boxTitleBtns);
            iconuser->setObjectName("iconuser");
            iconuser->setContentsMargins(0,0,0,2 * window->m_dpiRatio);
            iconuser->setMaximumWidth(200 * window->m_dpiRatio);
        }

        return iconuser;
    }

    void onScreenScalingFactor(int f)
    {
//        int _btncount = /*iconuser ? 4 :*/ 3;
//        int diffW = (titleLeftOffset - (TOOLBTN_WIDTH * _btncount)) * f; // 4 tool buttons: min+max+close+usericon

//        if ( iconuser ) {
//            iconuser->setContentsMargins(12*f,0,12*f,2*f);
//            iconuser->adjustSize();
//            diffW -= iconuser->width();
//        }

//        diffW > 0 ? window->m_labelTitle->setContentsMargins(0, 0, diffW, 2*f) :
//                        window->m_labelTitle->setContentsMargins(-diffW, 0, 0, 2*f);

//        for (auto btn: m_mapTitleButtons) {
//            btn->setFixedSize(QSize(TOOLBTN_WIDTH*f, TOOLBTN_HEIGHT*f));
//            btn->setIconSize(QSize(20,20) * f);
//        }
    }

};


#endif // CEDITORWINDOW2_P_H
