#ifndef CMAINWINDOWWRAP_H
#define CMAINWINDOWWRAP_H

//#define __WINWNDOW
//#define __WINWNDOW2
#define __WINWNDOW3
#if defined(__WINWNDOW)
#include "mainwindow.h"

class CMainWindowWrap : public CMainWindow
#elif defined(__WINWNDOW2)
#include "cmainwindow2.h"

class CMainWindowWrap : public CMainWindow2
#elif defined(__WINWNDOW3)
#include "cmainwindow3.h"

class CMainWindowWrap : public CMainWindow3
#endif
{
public:
    CMainWindowWrap(const QRect&);
};

#endif // CMAINWINDOWWRAP_H
