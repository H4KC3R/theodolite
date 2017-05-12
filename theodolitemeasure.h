#ifndef THEODOLITEMEASURE_H
#define THEODOLITEMEASURE_H
#include <tuple>
#include <QtGlobal>

class TheodoliteMeasure
{
public:
    explicit TheodoliteMeasure(const double& hz_angle, const double& v_angle, const quint16 circle);
    explicit TheodoliteMeasure();
    TheodoliteMeasure(const TheodoliteMeasure&)                   =default;
    TheodoliteMeasure(TheodoliteMeasure&& )                       =default;
    TheodoliteMeasure& operator=(const TheodoliteMeasure&)        =default;
    TheodoliteMeasure& operator=(TheodoliteMeasure&&)             =default;
    ~TheodoliteMeasure()                                          =default;
    void setMeasureData(const double& hz_angle, const double& v_angle, const quint16 circle);
    double getHzAngle() const;
    double getVAngle() const;
    quint16 getCircle() const;

private:
    std::tuple<double, double, quint16> measure_tuple;

};

#endif // THEODOLITEMEASURE_H
