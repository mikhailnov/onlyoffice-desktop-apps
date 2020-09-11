#ifndef CWINDOWPLATFORM_P_H
#define CWINDOWPLATFORM_P_H

#include "cwindowplatform.h"
#include <QDebug>

class CWindowPlatformPrivate
{
public:
    CWindowPlatformPrivate()
    {
    }

    virtual ~CWindowPlatformPrivate()
    {
    }

    auto init(CWindowPlatform * ptr)
    {
        q_ptr = ptr;
        m_hWnd = (HWND)q_ptr->winId();
    }

    auto handle()
    {
        return m_hWnd;
    }

    auto point_in_title(const QPoint& pt)
    {
        if ( m_titlebar && m_titlebar->rect().contains(pt) ) {
            QWidget * child = m_titlebar->childAt(pt);
            if ( !child || m_whiteList.contains(child) )
                return true;
        }

        return false;
    }

    auto set_title(QWidget * w)
    {
        m_titlebar = w;
    }

    virtual auto on_screen_scaling_changed(int f) -> void {
        m_dpiRatio = f;
    }

    CWindowPlatform * q_ptr;
    HWND m_hWnd = nullptr;

    QWidget * m_titlebar = nullptr;
    QList<QWidget*> m_whiteList;

    int m_dpiRatio = 1;
    bool is_window_custom_style = true;
};


#endif // CWINDOWPLATFORM_P_H
