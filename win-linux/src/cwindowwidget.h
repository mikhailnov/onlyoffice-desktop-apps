#ifndef CWINDOWWIDGET_H
#define CWINDOWWIDGET_H

//#include "cwindowbase.h"
#include <QMainWindow>

class CWindowWidget: /*public CWindowBase,*/ public QMainWindow
{
public:
    CWindowWidget(const QRect&);
    ~CWindowWidget() override;
};

#endif // CWINDOWWIDGET_H
