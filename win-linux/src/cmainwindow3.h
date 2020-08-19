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

    bool holdView(int id) const;
    auto editorsCount(const wstring&) const
    {
        return 1;
    }

    auto editorsCount() const
    {
        return 1;
    }

    auto attachEditor(QWidget *, int index = -1) -> int
    {
        return 0;
    }

    auto attachEditor(QWidget *, const QPoint&)
    {
        return 0;
    }

    auto bringToTop() {
        bringToFront();
    }

    auto windowRect() -> QRect {
        return geometry();
    }

    auto mainPanel() -> CMainPanel *;

    auto pointInTabs(const QPoint& pt) const
    {
        return true;
    }

    auto documentName(int) const
    {
        return QString("Test");
    }

    QWidget * editor(int)
    {
        return nullptr;
    }

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
