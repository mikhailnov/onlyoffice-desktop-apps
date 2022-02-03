#include "ctabbarwrapper.h"
#include <QDebug>

CTabBarWrapper::CTabBarWrapper(QWidget *parent):
    QFrame(parent)
{
    this->setObjectName(QString::fromUtf8("tabWrapper"));
    this->setFrameShape(QFrame::NoFrame);
    this->setFrameShadow(QFrame::Plain);

    QGridLayout *tabFrameLayout = new QGridLayout(this);
    tabFrameLayout->setSpacing(0);
    tabFrameLayout->setContentsMargins(0,0,0,0);
    this->setLayout(tabFrameLayout);

    // Bypassing the bug with tab scroller
    scrollerFrame = new QFrame(this);
    scrollerFrame->setObjectName("scrollerFrame");
    //scrollerFrame->setStyleSheet("QFrame {border: none; background: transparent;}");
    QHBoxLayout *scrollerLayout = new QHBoxLayout(scrollerFrame);
    scrollerLayout->setSpacing(0);
    scrollerLayout->setContentsMargins(0,0,0,0);
    scrollerFrame->setLayout(scrollerLayout);

    QToolButton* newLeftButton = new QToolButton(scrollerFrame);
    QToolButton* newRightButton = new QToolButton(scrollerFrame);
    newLeftButton->setObjectName("leftButton");
    newRightButton->setObjectName("rightButton");

    scrollerLayout->addWidget(newLeftButton);
    scrollerLayout->addWidget(newRightButton);
    newLeftButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    newRightButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    newLeftButton->installEventFilter(this);
    newRightButton->installEventFilter(this);
    newLeftButton->setMouseTracking(true);
    newRightButton->setMouseTracking(true);
    newLeftButton->setAttribute(Qt::WA_Hover, true);
    newRightButton->setAttribute(Qt::WA_Hover, true);   // End bypassing the bug

    bar = new CTabBar(this);
    bar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QWidget *paddingWidget = new QWidget(this);
    paddingWidget->setObjectName("paddingWidget");
    paddingWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QHBoxLayout *tabLayout = new QHBoxLayout(this);
#ifdef Q_OS_WIN
    tabLayout->setSpacing(32);
#else
    tabLayout->setSpacing(0);
#endif
    tabLayout->setContentsMargins(0,0,0,0);
    tabLayout->addWidget(bar);
    tabLayout->addWidget(paddingWidget);

    tabFrameLayout->addLayout(tabLayout, 0, 0, 1, 1);
    tabFrameLayout->addWidget(scrollerFrame, 0, 0, 1, 1, Qt::AlignRight);

    bar->initCustomScroll(scrollerFrame, newLeftButton, newRightButton);

}

CTabBarWrapper::~CTabBarWrapper()
{

}

CTabBar *CTabBarWrapper::tabBar()
{
    return bar;
}

void CTabBarWrapper::applyTheme(const QString &style)
{
    scrollerFrame->setStyleSheet(style);
    bar->setStyleSheet(style);
}

bool CTabBarWrapper::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::HoverEnter: {
        if (object->objectName() == QString("leftButton") ||
                object->objectName() == QString("rightButton")) {
            scrollerFrame->setCursor(QCursor(Qt::ArrowCursor));
        }
        break;
    }
    default:
        break;
    }
    return QWidget::eventFilter(object, event);
}
