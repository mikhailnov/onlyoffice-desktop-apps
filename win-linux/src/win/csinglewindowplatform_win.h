#ifndef CSINGLEWINDOWPLATFORM_WIN_H
#define CSINGLEWINDOWPLATFORM_WIN_H

#include <QMainWindow>
#include "csinglewindowbase.h"

#include <QAbstractNativeEventFilter>
class CAppNativeEventFilter: public QAbstractNativeEventFilter
{
public:
    virtual bool nativeEventFilter(const QByteArray & eventtype, void * message, long * result) override;
};


class CSingleWindowPlatform_win : public QMainWindow, public CSingleWindowBase
{
    Q_OBJECT

public:
    explicit CSingleWindowPlatform_win(const QRect& rect, const QString& title, QWidget * panel);
    virtual ~CSingleWindowPlatform_win() override;

    HWND handle() const;
    virtual void setWindowTitle(const QString &) override;
    virtual void show(bool showmaximized);
    virtual void bringToTop() override;

    auto setTitle(QWidget*) -> void;
    auto appendToTitle(QWidget*) -> void;
private:
    QWidget * m_titlebar = nullptr;
    QList<QWidget*> m_whiteList;

    QMargins m_frames;
    QMargins m_margins;

    bool m_bJustMaximized;

protected:
    virtual QWidget * createMainPanel(QWidget * parent, const QString& title, bool) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void onSizeEvent(int) override;
    virtual void onMinimizeEvent() override;
    virtual void onMaximizeEvent() override;
//    virtual void onExitSizeMove() override;
    virtual void setScreenScalingFactor(int) override;

    void captureMouse();

private:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
};

#endif // CSINGLEWINDOWPLATFORM_WIN_H
