
#include "cmainwindow3.h"
#include "cmainpanelimpl.h"
#include "cascapplicationmanagerwrapper.h"
#include <QGridLayout>

#ifdef Q_OS_WIN
# include "win/cwindowplatform_p.h"
#else
#endif

#define WINDOW_PRIVATE_CAST \
    CMainWindowPrivate * priv = reinterpret_cast<CMainWindowPrivate *>(d_pintf);


class CMainWindowPrivate : public CWindowPlatformPrivate
{
public:
    CMainWindowPrivate()
    {
    }

    virtual ~CMainWindowPrivate()
    {
        if ( m_pMainPanel ) {
            qDebug() << "mainpanel is exists";
        }
    }

    auto configure_title()
    {
        m_titlebar = m_pMainPanel;
        m_whiteList.append(m_pMainPanel->findChild<QLabel *>("labelAppTitle"));
    }

//    CMainWindow3 * q_ptr;
    CMainPanelImpl * m_pMainPanel = nullptr;
};


CMainWindow3::CMainWindow3(const QRect& rect)
    : CWindowPlatform(new CMainWindowPrivate, rect)
{
    CMainWindowPrivate * d_priv = reinterpret_cast<CMainWindowPrivate *>(d_pintf);
    d_priv->init(this);

    int m_dpiRatio = 1;

    QWidget * _widget = new QWidget(this);
    setCentralWidget(_widget);
    centralWidget()->setLayout(new QGridLayout);
    centralWidget()->layout()->setSpacing(0);
    centralWidget()->layout()->setMargin(4);

    d_priv->m_pMainPanel = new CMainPanelImpl(nullptr, true, m_dpiRatio);
    d_priv->m_pMainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));
    d_priv->m_pMainPanel->updateScaling(m_dpiRatio);

//    m_pMainPanel->goStart();

//    SetWindowPos(HWND(m_pWinPanel->winId()), NULL, 0, 0, _window_rect.width(), _window_rect.height(), SWP_FRAMECHANGED);

    CMainPanel * mainpanel = d_priv->m_pMainPanel;
    connect(mainpanel, &CMainPanel::mainWindowChangeState, this, &CMainWindow3::slot_windowChangeState);
    connect(mainpanel, &CMainPanel::mainWindowWantToClose, this, &CMainWindow3::slot_windowClose);
    connect(mainpanel, &CMainPanel::mainPageReady, this, &CMainWindow3::slot_mainPageReady);
//    QObject::connect(&AscAppManager::getInstance().commonEvents(), &CEventDriver::onModalDialog, bind(&CMainWindow::slot_modalDialog, this, _1, _2));

    centralWidget()->layout()->addWidget(mainpanel);
    d_priv->configure_title();
}

CMainWindow3::~CMainWindow3()
{
//    CMainWindowPrivate * d_priv = reinterpret_cast<CMainWindowPrivate *>(d_pintf);
//    delete d_priv->m_pMainPanel;
}

void CMainWindow3::slot_windowChangeState(Qt::WindowState s)
{
    int cmdShow = SW_RESTORE;
    switch (s) {
    case Qt::WindowMaximized:   cmdShow = SW_MAXIMIZE; break;
    case Qt::WindowMinimized:   cmdShow = SW_MINIMIZE; break;
    case Qt::WindowFullScreen:  cmdShow = SW_HIDE; break;
    default:
    case Qt::WindowNoState: break;
    }

//    ShowWindow(hWnd, cmdShow);
}

void CMainWindow3::slot_windowClose()
{
    AscAppManager::closeMainWindow(size_t(this));
}

void CMainWindow3::slot_mainPageReady()
{
//    CSplash::hideSplash();
}

auto CMainWindow3::mainPanel() -> CMainPanel *
{
    CMainWindowPrivate * d = reinterpret_cast<CMainWindowPrivate *>(d_pintf);
    return d->m_pMainPanel;
}

void CMainWindow3::show(bool)
{
    CWindowPlatform::show();
}

bool CMainWindow3::holdView(int id) const
{
    WINDOW_PRIVATE_CAST

    return priv->m_pMainPanel->holdUid(id);
}

auto CMainWindow3::editorsCount(const wstring& portal) const -> int
{
    WINDOW_PRIVATE_CAST

    return priv->m_pMainPanel->tabWidget()->count(portal);
}

auto CMainWindow3::editorsCount() const -> int
{
    WINDOW_PRIVATE_CAST

    return priv->m_pMainPanel->tabWidget()->count(cvwtEditor);
}

auto CMainWindow3::pointInTabs(const QPoint& pt) const -> bool
{
    WINDOW_PRIVATE_CAST

    QRect _rc_title(priv->m_pMainPanel->geometry());
    _rc_title.setHeight(priv->m_pMainPanel->tabWidget()->tabBar()->height());
    _rc_title.moveTop(1);

    return _rc_title.contains(priv->m_pMainPanel->mapFromGlobal(pt));
}

auto CMainWindow3::documentName(int id) const -> QString
{
    WINDOW_PRIVATE_CAST

    int i = priv->m_pMainPanel->tabWidget()->tabIndexByView(id);
    if ( !(i < 0) ) {
        return priv->m_pMainPanel->tabWidget()->panel(i)->data()->title();
    }

    return QString();
}

QWidget * CMainWindow3::editor(int index)
{
    WINDOW_PRIVATE_CAST

    return priv->m_pMainPanel->tabWidget()->panel(index);
}

auto CMainWindow3::attachEditor(QWidget * panel, int index) -> int
{
    CMainPanel * _pMainPanel = mainPanel();

    if (!QCefView::IsSupportLayers())
    {
        CTabPanel * _panel = dynamic_cast<CTabPanel *>(panel);
        if (_panel)
            _panel->view()->SetCaptionMaskSize(0);
    }

    int _index = _pMainPanel->tabWidget()->insertPanel(panel, index);
    if ( !(_index < 0) ) {
        _pMainPanel->toggleButtonMain(false);

        QTabBar * tabs = _pMainPanel->tabWidget()->tabBar();
        tabs->setCurrentIndex(_index);

//        if ( false ) {
//            QApplication::sendEvent( tabs,
//                &QMouseEvent(QEvent::MouseButtonPress,
//                    tabs->tabRect(_index).topLeft() + (QPoint(10, 65)*m_dpiRatio),
//                    Qt::LeftButton, Qt::LeftButton, Qt::NoModifier) );
//        }
    }

//    if (QApplication::mouseButtons().testFlag(Qt::LeftButton))
//        captureMouse(_index);

    return _index;
}

auto CMainWindow3::attachEditor(QWidget * panel, const QPoint& pt) -> int
{
    CMainPanel * _pMainPanel = mainPanel();
    QPoint _pt_local = _pMainPanel->tabWidget()->tabBar()->mapFromGlobal(pt);
#ifdef Q_OS_WIN
# if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    _pt_local -= geometry().topLeft();
# endif
#endif
    int _index = _pMainPanel->tabWidget()->tabBar()->tabAt(_pt_local);

    if ( !(_index < 0) ) {
        QRect _rc_tab = _pMainPanel->tabWidget()->tabBar()->tabRect(_index);
        if ( _pt_local.x() > _rc_tab.left() + (_rc_tab.width() / 2) ) ++_index;
    }

    return attachEditor(panel, _index);
}
