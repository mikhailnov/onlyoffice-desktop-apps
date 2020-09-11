
#include "cwindowplatform.h"
#include "cwindowplatform_p.h"

#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#pragma comment (lib,"Dwmapi.lib") // Adds missing library, fixes error LNK2019: unresolved external symbol __imp__DwmExtendFrameIntoClientArea
#pragma comment (lib,"user32.lib")

#include <QDebug>

bool CAppNativeEventFilter::nativeEventFilter(const QByteArray & eventtype, void * message, long * result)
{
    if ( eventtype == "windows_generic_MSG" ) {
        auto msg = *reinterpret_cast<MSG*>(message);
        if( msg.message == WM_NCHITTEST ) {
            CWindowPlatform * window = reinterpret_cast<CWindowPlatform *>(GetWindowLongPtr(msg.hwnd, GWLP_USERDATA));

            static int c = 0;
//            qDebug() << "native event" << ++c << QString(" 0x%1").arg(msg.message,4,16,QChar('0'));

qDebug() << "hittest" << ++c << window << (HWND)msg.hwnd;
            if( !window ) {
                auto root_wnd = GetAncestor(msg.hwnd, GA_ROOT);
                if ( root_wnd != GetDesktopWindow() ) {
                    window = reinterpret_cast<CWindowPlatform *>(GetWindowLongPtr(root_wnd, GWLP_USERDATA));
                    if ( window ) {
                        *result = HTTRANSPARENT;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}


CWindowPlatform::CWindowPlatform(const QRect& rect)
    : CWindowPlatform(new CWindowPlatformPrivate, rect)
{
}

CWindowPlatform::CWindowPlatform(CWindowPlatformPrivate *pp, const QRect& rect)
    : QMainWindow()
    , CWindowBase()
    , d_pintf(pp)
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

    HWND hwnd = (HWND)winId();
    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    //we better left 1 piexl width of border untouch, so OS can draw nice shadow around it
    const MARGINS shadow{1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(hwnd, &shadow);

    setGeometry(rect);
}

CWindowPlatform::~CWindowPlatform()
{
    qDebug() << "window platform destructor";
}

bool CWindowPlatform::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    #if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    MSG* msg = *reinterpret_cast<MSG**>(message);
    #else
    MSG* msg = reinterpret_cast<MSG*>(message);
    #endif

//    static int c = 0;
//    qDebug() << "native event" << ++c << QString(" 0x%1").arg(msg->message,4,16,QChar('0'));

    switch (msg->message) {
    case WM_NCCALCSIZE: {
        auto& params = *reinterpret_cast<NCCALCSIZE_PARAMS *>(msg->lParam);
//        if ( params.rgrc[0].top != 0 )
//            params.rgrc[0].top -= 1;

        //this kills the window frame and title bar we added with WS_THICKFRAME and WS_CAPTION
        *result = WVR_REDRAW;
//        *result = 0;
        return true; }

    case WM_NCHITTEST: {
        *result = 0;
        qDebug() << "WM_NCHITTEST";

        const LONG border_width = 8;
        RECT winrect;
        GetWindowRect(msg->hwnd, &winrect);

        long x = GET_X_LPARAM(msg->lParam);
        long y = GET_Y_LPARAM(msg->lParam);

//        if(m_bResizeable)
        {

            bool resizeWidth = minimumWidth() != maximumWidth();
            bool resizeHeight = minimumHeight() != maximumHeight();

            if( resizeWidth ) {
                //left border
                if (x >= winrect.left && x < winrect.left + border_width)
                    *result = HTLEFT;

                //right border
                if (x < winrect.right && x >= winrect.right - border_width) {
                    *result = HTRIGHT;
                }
            }
            if( resizeHeight ) {
                //bottom border
                if (y < winrect.bottom && y >= winrect.bottom - border_width)
                {
                    *result = HTBOTTOM;
                }
                //top border
                if (y >= winrect.top && y < winrect.top + border_width)
                {
                    *result = HTTOP;
                }
            }
            if( resizeWidth && resizeHeight ) {
                //bottom left corner
                if (x >= winrect.left && x < winrect.left + border_width &&
                        y < winrect.bottom && y >= winrect.bottom - border_width)
                {
                    *result = HTBOTTOMLEFT;
                }
                //bottom right corner
                if (x < winrect.right && x >= winrect.right - border_width &&
                        y < winrect.bottom && y >= winrect.bottom - border_width)
                {
                    *result = HTBOTTOMRIGHT;
                }
                //top left corner
                if (x >= winrect.left && x < winrect.left + border_width &&
                        y >= winrect.top && y < winrect.top + border_width)
                {
                    *result = HTTOPLEFT;
                }
                //top right corner
                if (x < winrect.right && x >= winrect.right - border_width &&
                        y >= winrect.top && y < winrect.top + border_width)
                {
                    *result = HTTOPRIGHT;
                }
            }
        }

        if ( 0 != *result ) {
            return true;
        }

        if ( d_pintf->m_titlebar ) {
            //support highdpi
            double dpr = this->devicePixelRatioF();
            QPoint pos = d_pintf->m_titlebar->mapFromGlobal(QPoint(x/dpr,y/dpr));

            if ( d_pintf->point_in_title(pos) ) {
                *result = HTCAPTION;
                return true;
            }
        }

        return false;
    }

    case WM_GETMINMAXINFO: {
        if ( ::IsZoomed(msg->hwnd) ) {
            RECT frame{0, 0, 0, 0};
            AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);

            //record frame area data
            double dpr = this->devicePixelRatioF();

            m_frames.setLeft(abs(frame.left)/dpr);
            m_frames.setTop(abs(frame.bottom)/dpr);
            m_frames.setRight(abs(frame.right)/dpr);
            m_frames.setBottom(abs(frame.bottom)/dpr);

            QMainWindow::setContentsMargins(m_frames.left() + m_margins.left(), m_frames.top() + m_margins.top(),
                                            m_frames.right() + m_margins.right(), m_frames.bottom() + m_margins.bottom());
            m_bJustMaximized = true;
        } else {
            if (m_bJustMaximized)
            {
                QMainWindow::setContentsMargins(m_margins);
                m_frames = QMargins();
                m_bJustMaximized = false;
            }
        }
        return false;
    }

    default: return QMainWindow::nativeEvent(eventType, message, result);
    }
}

void CWindowPlatform::bringToFront() const
{
    if (IsIconic(d_pintf->handle())) {
        ShowWindow(d_pintf->handle(), SW_SHOWNORMAL);
    }

    SetForegroundWindow(d_pintf->handle());
    SetFocus(d_pintf->handle());
    SetActiveWindow(d_pintf->handle());
}

WindowNativeHandle CWindowPlatform::handle() const
{
    return d_pintf->handle();
}
