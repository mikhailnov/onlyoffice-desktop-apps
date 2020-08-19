#include "cmainwindowwrap.h"

CMainWindowWrap::CMainWindowWrap(const QRect& rect)
#ifdef __WINWNDOW
    : CMainWindow(rect)
#elif defined (__WINWNDOW2)
    : CMainWindow2(rect)
#elif defined (__WINWNDOW3)
    : CMainWindow3(rect)
#endif
{

}
