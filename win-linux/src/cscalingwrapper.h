#ifndef CSCALINGWRAPPER_H
#define CSCALINGWRAPPER_H

#include <QWidget>

class CScalingWrapper
{
public:
    explicit CScalingWrapper(QWidget *);
    explicit CScalingWrapper(double);
    virtual ~CScalingWrapper() {}

    virtual void updateScaling(double f);
    double scaling() const;

    static void updateChildScaling(const QObject * parent, double factor);
    static double parentScalingFactor(const QObject * parent);

private:
    double m_scaleFactor = 1.0;
};

#endif // CSCALINGWRAPPER_H
