#include "theodolitemeasure.h"

TheodoliteMeasure::TheodoliteMeasure()
{
     measure_tuple=std::make_tuple(0,0,0);
}

TheodoliteMeasure::TheodoliteMeasure(const double& hz_angle, const double& v_angle, const quint16 circle)
{
    measure_tuple=std::make_tuple(hz_angle,v_angle,circle+1);
}

void TheodoliteMeasure::setMeasureData(const double& hz_angle, const double& v_angle, const quint16 circle)
{
    measure_tuple=std::make_tuple(hz_angle,v_angle,circle+1);
}


double TheodoliteMeasure::getHzAngle() const
{
    return std::get<0> (measure_tuple);
}
double TheodoliteMeasure::getVAngle() const
{
    return std::get<1> (measure_tuple);
}
quint16 TheodoliteMeasure::getCircle() const
{
    return std::get<2>(measure_tuple);
}
