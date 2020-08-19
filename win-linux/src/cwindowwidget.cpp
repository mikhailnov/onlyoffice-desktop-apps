
#include "cwindowwidget.h"
#include "cwindowbase.h"
#include "utils.h"

CWindowWidget::CWindowWidget(const QRect& rect)
    : QMainWindow()
{
    QRect window_rect_ = rect;
//    m_dpiRatio = CSplash::startupDpiRatio();
    int m_dpiRatio = 1;

    if ( window_rect_.isEmpty() )
        window_rect_ = QRect(QPoint(100, 100)*m_dpiRatio, QSize(MAIN_WINDOW_START_WIDTH, MAIN_WINDOW_START_HEIGHT)*m_dpiRatio);

    QRect screen_size_ = Utils::getScreenGeometry(window_rect_.topLeft());
    if ( screen_size_.intersects(window_rect_) ) {
        if ( screen_size_.width() < window_rect_.width() ||
                screen_size_.height() < window_rect_.height() )
        {
            window_rect_.setLeft(screen_size_.left()),
            window_rect_.setTop(screen_size_.top());

            if ( screen_size_.width() < window_rect_.width() ) window_rect_.setWidth(screen_size_.width());
            if ( screen_size_.height() < window_rect_.height() ) window_rect_.setHeight(screen_size_.height());
        }
    } else {
        window_rect_ = QRect(QPoint(100, 100)*m_dpiRatio, QSize(MAIN_WINDOW_MIN_WIDTH, MAIN_WINDOW_MIN_HEIGHT)*m_dpiRatio);
    }

    setGeometry(window_rect_);
//    setWindowIcon(Utils::appIcon());
}

CWindowWidget::~CWindowWidget()
{

}
