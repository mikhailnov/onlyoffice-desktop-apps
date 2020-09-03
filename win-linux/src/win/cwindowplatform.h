#ifndef CWINDOWPLATFORM_H
#define CWINDOWPLATFORM_H

#include "cwindowwidget.h"
#include "cwindowbase.h"
#include <memory>

#include <QAbstractNativeEventFilter>
class CAppNativeEventFilter: public QAbstractNativeEventFilter
{
public:
    virtual bool nativeEventFilter(const QByteArray &, void *, long *) override;
};


class CWindowPlatformPrivate;
class CWindowPlatform: public QMainWindow, public CWindowBase
{
    friend class CWindowPlatformPrivate;

public:
    CWindowPlatform(const QRect&);
    virtual ~CWindowPlatform() override;

    virtual void bringToFront() const override;
    virtual WindowNativeHandle handle() const override;

protected:
    CWindowPlatform(CWindowPlatformPrivate *, const QRect&);
    CWindowPlatformPrivate * d_pintf;

private:
    bool nativeEvent(const QByteArray &, void *, long *) override;

private:
    QMargins m_frames;
    QMargins m_margins;

    bool m_bJustMaximized;
};

#endif // CWINDOWPLATFORM_H
