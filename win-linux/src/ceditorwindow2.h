#ifndef CEDITORWINDOW2_H
#define CEDITORWINDOW2_H

#include <QWidget>
#include "ctabpanel.h"

#ifdef Q_OS_WIN
# include "win/cwindowplatform.h"
#endif

class CEditorWindow2Private;
class CEditorWindow2: public CWindowPlatform
{
    friend class CEditorWindow2Private;

public:
    CEditorWindow2(const QRect&, CTabPanel *);

    /* TODO: documentName -> documentTitle
     *      documentName -> editor().title() */
    auto documentName() const -> QString;
    auto show(bool) -> void;
    void undock(bool maximized = false);
    int closeWindow();

    auto setReporterMode(bool) {}
    auto closed() const
    {
        return false;
    }
};

#endif // CEDITORWINDOW2_H
