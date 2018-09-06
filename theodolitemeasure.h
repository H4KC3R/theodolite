#ifndef THEODOLITEMEASURE_H
#define THEODOLITEMEASURE_H
#include <tuple>
#include <QtGlobal>

class TheodoliteMeasure
{
public:
    explicit TheodoliteMeasure(const double& hzAngle, const double& Vangle, const qint32 circle);

    explicit TheodoliteMeasure();

    TheodoliteMeasure(const TheodoliteMeasure&)                   =default;

    TheodoliteMeasure(TheodoliteMeasure&& )                       =default;

    TheodoliteMeasure& operator=(const TheodoliteMeasure&)        =default;

    TheodoliteMeasure& operator=(TheodoliteMeasure&&)             =default;

    ~TheodoliteMeasure()                                          =default;

    void setMeasureData(const double& hzAngle, const double& Vangle, const qint32 circle);

    double getHzAngle() const;

    double getVAngle() const;

    qint32 getCircle() const;

private:
    std::tuple <double, double, qint32> measureTuple;

};

#endif // THEODOLITEMEASURE_H
