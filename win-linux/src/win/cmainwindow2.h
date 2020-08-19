#ifndef CMAINWINDOW2_H
#define CMAINWINDOW2_H

#include "cmainwindowbase.h"
#include "csinglewindowplatform_win.h"
#include "cmainpanelimpl.h"

class CMainWindow2 : public CMainWindowBase, public CSingleWindowPlatform_win
{
public:
    CMainWindow2(const QRect&);

public:
    CMainPanelImpl * m_pMainPanel = nullptr;

public slots:
    void slot_windowChangeState(Qt::WindowState);
    void slot_windowClose();
    void slot_mainPageReady();

public:
    CMainPanel * mainPanel() const override;
    QRect windowRect() const override;
    bool holdView(int) const override;
    void bringToTop() const override;
    bool isMaximized() const override;
private:
    using CSingleWindowBase::holdView;
//    using CSingleWindowBase::bringToTop;

protected:
    void onMoveEvent(const QRect&) override;

};

#endif // CMAINWINDOW2_H
