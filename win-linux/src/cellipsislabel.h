#ifndef CELLIPSISLABEL_H
#define CELLIPSISLABEL_H

#include <QLabel>

class CEllipsisLabel: public QLabel
{
public:
    CEllipsisLabel(const QString &text, QWidget *parent=Q_NULLPTR);
    explicit CEllipsisLabel(QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());

    auto setText(const QString&) -> void;
    auto setEllipsisMode(Qt::TextElideMode) -> void;
protected:
    void resizeEvent(QResizeEvent *event) override;

    using QLabel::setText;
private:
    QString orig_text;
    Qt::TextElideMode elide_mode = Qt::ElideRight;
};

#endif // CELLIPSISLABEL_H
