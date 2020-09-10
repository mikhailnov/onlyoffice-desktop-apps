
#include "ceditorwindow2.h"
#include "ceditorwindow2_p.h"
#include "cmessage.h"
#include "defines.h"
#include "cascapplicationmanagerwrapper.h"

#define WINDOW_PRIVATE_CAST \
    CEditorWindow2Private * pintf = reinterpret_cast<CEditorWindow2Private *>(d_pintf);


CEditorWindow2::CEditorWindow2(const QRect& rect, CTabPanel * panel)
    : CWindowPlatform(new CEditorWindow2Private, rect)
{
    WINDOW_PRIVATE_CAST

    pintf->init(this, panel);

#ifdef Q_OS_LINUX
    setObjectName("editorWindow");
    m_pMainPanel = createMainPanel(this);
    setCentralWidget(m_pMainPanel);

    if ( !CX11Decoration::isDecorated() ) {
        CX11Decoration::setTitleWidget(m_boxTitleBtns);
        m_pMainPanel->setMouseTracking(true);
        setMouseTracking(true);
    }
#else

    if ( pintf->extendableTitle() ) {
//        QColor color;
//        switch (panel->data()->contentType()) {
//        case etDocument: color = QColor(TAB_COLOR_DOCUMENT); break;
//        case etPresentation: color = QColor(TAB_COLOR_PRESENTATION); break;
//        case etSpreadsheet: color = QColor(TAB_COLOR_SPREADSHEET); break;
//        }

//        m_bgColor = RGB(color.red(), color.green(), color.blue());
    }

    pintf->m_pMainPanel = createMainPanel();
    setCentralWidget(pintf->m_pMainPanel);

    pintf->configure_title();
    pintf->on_window_resize();
#endif

//    QTimer::singleShot(0, [=]{m_pMainView->show();});
    AscAppManager::bindReceiver(panel->cef()->GetId(), static_cast<CCefEventsGate * const>(&(pintf->m_listener)));
    AscAppManager::sendCommandTo(panel->cef(), L"editor:config", L"request");
}

auto CEditorWindow2::documentName() const -> QString
{
    return "";
}

auto CEditorWindow2::editorType() const -> AscEditorType
{
    return AscEditorType::etDocument;
}

auto CEditorWindow2::holdView(const wstring&) const -> bool
{
    return true;
}

auto CEditorWindow2::holdView(int) const -> bool
{
    return true;
}

CTabPanel * CEditorWindow2::releaseEditorView() const
{
//    m_pMainView->clearMask();
//    return qobject_cast<CTabPanel *>(m_pMainView);
    return nullptr;
}

const QObject * CEditorWindow2::receiver()
{
    WINDOW_PRIVATE_CAST

    return &pintf->m_listener;
}

auto CEditorWindow2::show(bool) -> void
{
    CWindowPlatform::show();
}

void CEditorWindow2::undock(bool maximized)
{
#ifdef Q_OS_LINUX
    maximized = false;
#else
    if ( maximized ) {
//        m_restoreMaximized = true;
        maximized = false;
    }
#endif

//    CSingleWindowPlatform_win::show(maximized);
//    CSingleWindowPlatform_win::captureMouse();
    CWindowPlatform::show();
}

int CEditorWindow2::closeWindow()
{
//    d_ptr.get()->onFullScreen(false);

//    CTabPanel * panel = d_ptr.get()->panel();

//    int _reply = MODAL_RESULT_YES;
//    if ( panel->data()->hasChanges() && !panel->data()->closed() ) {
//        if (windowState() == Qt::WindowMinimized)
//            setWindowState(Qt::WindowNoState);

//        bringToFront();

//        CMessage mess(handle(), CMessageOpts::moButtons::mbYesDefNoCancel);
////            modal_res = mess.warning(getSaveMessage().arg(m_pTabs->titleByIndex(index)));
//        _reply = mess.warning(tr("%1 has been changed. Save changes?").arg(panel->data()->title(true)));

//        switch (_reply) {
//        case MODAL_RESULT_CUSTOM + 1:
//            _reply = MODAL_RESULT_YES;
//            break;
//        case MODAL_RESULT_CANCEL:
//        case MODAL_RESULT_CUSTOM + 2:
//            return MODAL_RESULT_CANCEL;

//        case MODAL_RESULT_CUSTOM + 0:
//        default:
//            panel->data()->close();
//            panel->cef()->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE));

//            _reply = MODAL_RESULT_NO;
//            break;
//        }
//    }

//    if ( _reply == MODAL_RESULT_YES ) {
//        panel->data()->close();
//        d_ptr.get()->onDocumentSave(panel->cef()->GetId());
//    }

//    return _reply;
    return MODAL_RESULT_YES;
}

auto CEditorWindow2::createMainPanel() -> QWidget *
{
    WINDOW_PRIVATE_CAST

    // create min/max/close buttons
    pintf->create_custom_elements();

    QWidget * mainPanel = new QWidget(this);
    mainPanel->setObjectName("mainPanel");

    QGridLayout * mainGridLayout = new QGridLayout();
    mainGridLayout->setSpacing(0);
#ifdef Q_OS_WIN
    mainGridLayout->setMargin(8);
    mainGridLayout->setContentsMargins(QMargins(4,4,4,4) * pintf->m_dpiRatio);
#else
    int b = CX11Decoration::customWindowBorderWith() * m_dpiRatio;
    mainGridLayout->setContentsMargins(QMargins(b,b,b,b));
#endif
    mainPanel->setLayout(mainGridLayout);

    if ( pintf->m_dpiRatio > 1 )
        mainPanel->setProperty("zoom", "2x");

    QString css(AscAppManager::getWindowStylesheets(pintf->m_dpiRatio));
    css.append(pintf->m_css);
    mainPanel->setStyleSheet(css);

    if ( !pintf->extendableTitle() ) {
        mainGridLayout->addWidget(pintf->m_boxTitleBtns);
        pintf->m_labelTitle->setText(APP_TITLE);
    } else {
        mainPanel->setProperty("window", "pretty");
        pintf->m_boxTitleBtns->setParent(mainPanel);
        pintf->m_boxTitleBtns->layout()->addWidget(pintf->iconUser());
    }

    if ( /*custom*/ true ) {
        pintf->m_boxTitleBtns->layout()->addWidget(pintf->m_buttonMinimize);
        pintf->m_boxTitleBtns->layout()->addWidget(pintf->m_buttonMaximize);
        pintf->m_boxTitleBtns->layout()->addWidget(pintf->m_buttonClose);
    } else {
    }

    if ( !pintf->panel() ) {
//        QCefView * pMainWidget = AscAppManager::createViewer(centralWidget);
//        pMainWidget->Create(&AscAppManager::getInstance(), cvwtSimple);
//        pMainWidget->setObjectName( "mainPanel" );
//        pMainWidget->setHidden(false);

//        m_pMainView = (QWidget *)pMainWidget;
    } else {
    }

//    m_pMainWidget->setVisible(false);

//    pintf->on_screen_scaling_changed(pintf->m_dpiRatio);
    mainGridLayout->addWidget(pintf->panel(), 1, 0);
    mainGridLayout->setRowStretch(1,1);
    return mainPanel;
}

void CEditorWindow2::resizeEvent(QResizeEvent *event)
{
    WINDOW_PRIVATE_CAST

    pintf->on_window_resize();
}
