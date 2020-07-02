
#include "csinglewindowplatform_win.h"

#include <Windows.h>
//#include <WinUser.h>
#include <windowsx.h>
#include <dwmapi.h>
//#include <objidl.h> // Fixes error C2504: 'IUnknown' : base class undefined
//#include <gdiplus.h>
//#include <GdiPlusColor.h>
#pragma comment (lib,"Dwmapi.lib") // Adds missing library, fixes error LNK2019: unresolved external symbol __imp__DwmExtendFrameIntoClientArea
#pragma comment (lib,"user32.lib")

#include "utils.h"
#include <QDebug>
#include <QEvent>
#include <QApplication>

#include <QtGui/qpa/qplatformnativeinterface.h>
static QWindow* windowForWidget(const QWidget* widget)
{
       QWindow* window = widget->windowHandle();
       if (window)
           return window;
       const QWidget* nativeParent = widget->nativeParentWidget();
       if (nativeParent)
           return nativeParent->windowHandle();
       return 0;
}

HWND getHWNDForWidget(const QWidget* widget)
{
       QWindow* window = ::windowForWidget(widget);
       if (window)
       {
           QPlatformNativeInterface* interfacep = QGuiApplication::platformNativeInterface();
           return static_cast<HWND>(interfacep->nativeResourceForWindow(QByteArrayLiteral("handle"), window));
       }
       return 0;
}


HWND g_testHandle = 0;
bool CAppNativeEventFilter::nativeEventFilter(const QByteArray & eventtype, void * message, long * result)
{
    if ( eventtype == "windows_generic_MSG" ) {
        auto msg = *reinterpret_cast<MSG*>(message);
        if( msg.message == WM_NCHITTEST ) {
            if (msg.hwnd == g_testHandle)  {
                *result = HTTRANSPARENT;
                return false;
            }

            auto window = reinterpret_cast<CSingleWindowPlatform_win *>(GetWindowLongPtr(msg.hwnd, GWLP_USERDATA));

            static int c = 0;
//            qDebug() << "native event" << ++c << QString(" 0x%1").arg(msg.message,4,16,QChar('0'));

//qDebug() << "hittest" << ++c << window << ownd;
            if( !window ) {
                HWND parent_wnd = GetAncestor(msg.hwnd, GA_PARENT);
                if ( parent_wnd == g_testHandle ) {
                    qDebug() << "has no owner 2" << ++c << msg.hwnd;
                    *result = HTTRANSPARENT;
                    return true;
                }

                HWND root_wnd = GetAncestor(msg.hwnd, GA_ROOT);

                if ( root_wnd != GetDesktopWindow() )
                {
                    window = reinterpret_cast<CSingleWindowPlatform_win *>(GetWindowLongPtr(root_wnd, GWLP_USERDATA));
//                    qDebug() << "has no owner" << ++c << msg.hwnd << window;

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

CSingleWindowPlatform_win::CSingleWindowPlatform_win(const QRect& rect, const QString&, QWidget *)
    : QMainWindow()
    , CSingleWindowBase(const_cast<QRect&>(rect))
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

    //this line will get titlebar/thick frame/Aero back, which is exactly what we want
    //we will get rid of titlebar and thick frame again in nativeEvent() later
    HWND hwnd = (HWND)winId();
    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    //we better left 1 piexl width of border untouch, so OS can draw nice shadow around it
    const MARGINS shadow = { 1, 1, 1, 1 };
    DwmExtendFrameIntoClientArea(hwnd, &shadow);

    setWindowIcon(Utils::appIcon());
    setGeometry(rect);

    QApplication::instance()->installNativeEventFilter(new CAppNativeEventFilter);
}

CSingleWindowPlatform_win::~CSingleWindowPlatform_win()
{

}

bool CSingleWindowPlatform_win::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    #if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    MSG* msg = *reinterpret_cast<MSG**>(message);
    #else
    MSG* msg = reinterpret_cast<MSG*>(message);
    #endif

    static int c = 0;
//    qDebug() << "native event" << ++c << QString(" 0x%1").arg(msg->message,4,16,QChar('0'));

    switch (msg->message) {

    case WM_NCCALCSIZE: {
        auto& params = *reinterpret_cast<NCCALCSIZE_PARAMS *>(msg->lParam);
        if ( params.rgrc[0].top != 0 )
            params.rgrc[0].top -= 1;

        //this kills the window frame and title bar we added with WS_THICKFRAME and WS_CAPTION
        *result = WVR_REDRAW;
        return true; }

    case WM_NCHITTEST: {
        *result = 0;
        qDebug() << "WM_NCHITTEST";

        const LONG border_width = 8;
        RECT winrect;
        GetWindowRect(HWND(winId()), &winrect);

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

        if ( 0 != *result ) return true;

        //*result still equals 0, that means the cursor locate OUTSIDE the frame area
        //but it may locate in titlebar area
        if ( !m_titlebar ) return false;

        //support highdpi
        double dpr = this->devicePixelRatioF();
        QPoint pos = m_titlebar->mapFromGlobal(QPoint(x/dpr,y/dpr));

        if (!m_titlebar->rect().contains(pos)) return false;
        QWidget* child = m_titlebar->childAt(pos);
        if (!child)
        {
            *result = HTCAPTION;
            return true;
        }else{
            if (m_whiteList.contains(child))
            {
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

QWidget * CSingleWindowPlatform_win::createMainPanel(QWidget * parent, const QString& title, bool iscustom)
{
    CSingleWindowBase::createMainPanel(parent, title, iscustom);
    m_titlebar = CSingleWindowBase::m_boxTitleBtns;
    m_whiteList.append(m_labelTitle);

//    g_testHandle = (HWND)m_boxTitleBtns->winId();
    return nullptr;
}

void CSingleWindowPlatform_win::resizeEvent(QResizeEvent *)
{
    onSizeEvent(0);
}

void CSingleWindowPlatform_win::onSizeEvent(int type)
{
    CSingleWindowBase::onSizeEvent(type);
}

void CSingleWindowPlatform_win::onMinimizeEvent()
{
//    ShowWindow(m_hWnd, SW_MINIMIZE);
    setWindowState(Qt::WindowMinimized);
}

void CSingleWindowPlatform_win::onMaximizeEvent()
{
//    ShowWindow(m_hWnd, IsZoomed(m_hWnd) ? SW_RESTORE : SW_MAXIMIZE);
    setWindowState(Qt::WindowMaximized);
}

void CSingleWindowPlatform_win::setWindowTitle(const QString& t)
{
    CSingleWindowBase::setWindowTitle(t);
    QMainWindow::setWindowTitle(t);
}

void CSingleWindowPlatform_win::setScreenScalingFactor(int f)
{

}

HWND CSingleWindowPlatform_win::handle() const
{
    return (HWND)winId();
}

void CSingleWindowPlatform_win::captureMouse()
{

}

void CSingleWindowPlatform_win::show(bool maximized)
{
//    ShowWindow(m_hWnd, maximized ? SW_MAXIMIZE : SW_SHOW);
////    UpdateWindow(m_hWnd);
//    m_visible = true;
    QMainWindow::show();
}

void CSingleWindowPlatform_win::bringToTop()
{
//    SetForegroundWindow(m_hWnd);
//    SetFocus(m_hWnd);
//    SetActiveWindow(m_hWnd);
}
