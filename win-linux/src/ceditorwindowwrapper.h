#ifndef CEDITORWINDOWWRAPPER_H
#define CEDITORWINDOWWRAPPER_H

#define EDITOR_WINDOW1
#ifndef EDITOR_WINDOW1
# define EDITOR_WINDOW2
#endif

#ifdef EDITOR_WINDOW1
# include "ceditorwindow.h"

class CEditorWindowWrap: public CEditorWindow
#elif defined(EDITOR_WINDOW2)
# include "ceditorwindow2.h"

class CEditorWindowWrap: public CEditorWindow2
#endif
{
public:
    CEditorWindowWrap(const QRect& rect, CTabPanel* view);

#if defined(EDITOR_WINDOW2)
    auto bringToTop() const {
        CEditorWindow2::bringToFront();
    }
#endif

};

#endif // CEDITORWINDOWWRAPPER_H
