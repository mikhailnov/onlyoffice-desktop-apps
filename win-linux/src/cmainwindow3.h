#ifndef CMAINWINDOW3_H
#define CMAINWINDOW3_H

#include <QWidget>
#include "win/cwindowplatform.h"
#include "cmainpanelimpl.h"

class CMainWindowPrivate;
class CMainWindow3: public CWindowPlatform
{
public:
    CMainWindow3(const QRect& rect);
    ~CMainWindow3() override;

    bool holdView(int id) const override;
    auto editorsCount(const wstring&) const -> int;
    auto editorsCount() const -> int;
    auto mainPanel() -> CMainPanel *;
    auto attachEditor(QWidget *, int index = -1) -> int;
    auto attachEditor(QWidget *, const QPoint&) -> int;
    auto pointInTabs(const QPoint& pt) const -> bool;
    auto documentName(int) const -> QString;
    auto editor(int) -> QWidget *;

    void show(bool);


public slots:
    void slot_windowChangeState(Qt::WindowState);
    void slot_windowClose();
    void slot_mainPageReady();

private:
    friend class CMainWindowPrivate;
//    CMainWindowPrivate * d_priv;
};

#endif // CMAINWINDOW3_H
