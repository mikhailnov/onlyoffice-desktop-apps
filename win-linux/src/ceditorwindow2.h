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
    auto editorType() const -> AscEditorType;
    auto holdView(const wstring&) const -> bool;
    auto holdView(int) const -> bool override;
    CTabPanel * releaseEditorView() const;


    const QObject * receiver();

    auto show(bool) -> void;
    void undock(bool maximized = false);
    int closeWindow();

    auto setReporterMode(bool) {}
    auto closed() const
    {
        return false;
    }

private:
    auto createMainPanel() -> QWidget *;
};

#endif // CEDITORWINDOW2_H
