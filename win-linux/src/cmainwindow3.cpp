
#include "cmainwindow3.h"
#include "cmainpanelimpl.h"
#include "cascapplicationmanagerwrapper.h"
#include <QGridLayout>

#ifdef Q_OS_WIN
# include "win/cwindowplatform_p.h"
#else
#endif

class CMainWindowPrivate : public CWindowPlatformPrivate
{
public:
    CMainWindowPrivate()
    {
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
//    d_pintf->init(this);
//    CMainWindowPrivate * d_priv = reinterpret_cast<CMainWindowPrivate *>(d_pintf);

    int m_dpiRatio = 1;

    QWidget * _widget = new QWidget(this);
    setCentralWidget(_widget);
    centralWidget()->setLayout(new QGridLayout);
    centralWidget()->layout()->setSpacing(0);
    centralWidget()->layout()->setMargin(4);

    CMainPanelImpl * m_pMainPanel = new CMainPanelImpl(nullptr, true, m_dpiRatio);
    m_pMainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));
    m_pMainPanel->updateScaling(m_dpiRatio);

//    m_pMainPanel->goStart();

//    SetWindowPos(HWND(m_pWinPanel->winId()), NULL, 0, 0, _window_rect.width(), _window_rect.height(), SWP_FRAMECHANGED);

    CMainPanel * mainpanel = m_pMainPanel;
    connect(mainpanel, &CMainPanel::mainWindowChangeState, this, &CMainWindow3::slot_windowChangeState);
    connect(mainpanel, &CMainPanel::mainWindowWantToClose, this, &CMainWindow3::slot_windowClose);
    connect(mainpanel, &CMainPanel::mainPageReady, this, &CMainWindow3::slot_mainPageReady);
//    QObject::connect(&AscAppManager::getInstance().commonEvents(), &CEventDriver::onModalDialog, bind(&CMainWindow::slot_modalDialog, this, _1, _2));

    centralWidget()->layout()->addWidget(mainpanel);
//    d_priv->configure_title();
}

CMainWindow3::~CMainWindow3()
{

}

bool CMainWindow3::holdView(int) const
{
    return true;
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
