
#include "cmainwindow2.h"
#include "cascapplicationmanagerwrapper.h"

CMainWindow2::CMainWindow2(const QRect& rect)
    : CMainWindowBase()
    , CSingleWindowPlatform_win(rect, "Main Window", nullptr)
{
    QWidget * _widget = new QWidget(this);
    setCentralWidget(_widget);
    centralWidget()->setLayout(new QGridLayout);
    centralWidget()->layout()->setSpacing(0);
    centralWidget()->layout()->setMargin(4);

    m_pMainPanel = new CMainPanelImpl(nullptr, true, m_dpiRatio);
    m_pMainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));
    m_pMainPanel->updateScaling(m_dpiRatio);

//    m_pMainPanel->goStart();

//    SetWindowPos(HWND(m_pWinPanel->winId()), NULL, 0, 0, _window_rect.width(), _window_rect.height(), SWP_FRAMECHANGED);

    CMainPanel * mainpanel = m_pMainPanel;
    QObject::connect(mainpanel, &CMainPanel::mainWindowChangeState, this, &CMainWindow2::slot_windowChangeState);
    QObject::connect(mainpanel, &CMainPanel::mainWindowWantToClose, this, &CMainWindow2::slot_windowClose);
    QObject::connect(mainpanel, &CMainPanel::mainPageReady, this, &CMainWindow2::slot_mainPageReady);
//    QObject::connect(&AscAppManager::getInstance().commonEvents(), &CEventDriver::onModalDialog, bind(&CMainWindow::slot_modalDialog, this, _1, _2));

    centralWidget()->layout()->addWidget(m_pMainPanel);
    setTitle(m_pMainPanel);
    appendToTitle(m_pMainPanel->findChild<QLabel *>("labelAppTitle"));
}

void CMainWindow2::slot_windowChangeState(Qt::WindowState s)
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

void CMainWindow2::slot_windowClose()
{
    AscAppManager::closeMainWindow(size_t(this));
}

void CMainWindow2::slot_mainPageReady()
{
//    CSplash::hideSplash();
}

bool CMainWindow2::holdView(int id) const
{
    return CMainWindowBase::holdView(id);
}

void CMainWindow2::bringToTop() const
{
//    CMainWindowBase::bringToTop();
}

void CMainWindow2::onMoveEvent(const QRect&)
{

}

bool CMainWindow2::isMaximized() const
{
    return false;
}

CMainPanel * CMainWindow2::mainPanel() const
{
    return m_pMainPanel;
}

QRect CMainWindow2::windowRect() const
{
    return geometry();
}
