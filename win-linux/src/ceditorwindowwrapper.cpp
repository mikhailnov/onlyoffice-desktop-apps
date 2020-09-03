#include "ceditorwindowwrapper.h"

CEditorWindowWrap::CEditorWindowWrap(const QRect& rect, CTabPanel* view)
#ifdef EDITOR_WINDOW1
    : CEditorWindow(rect, view)
#elif defined (EDITOR_WINDOW2)
    : CEditorWindow2(rect, view)
#endif
{

}
