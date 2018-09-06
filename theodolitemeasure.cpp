#include "theodolitemeasure.h"

TheodoliteMeasure::TheodoliteMeasure()
{
     measureTuple = std::make_tuple(0, 0, 0);
}

TheodoliteMeasure::TheodoliteMeasure(const double& hzAngle, const double& Vangle, const qint32 circle)
{
    measureTuple = std::make_tuple(hzAngle, Vangle,circle+1);
}

void TheodoliteMeasure::setMeasureData(const double& hzAngle, const double& Vangle, const qint32 circle)
{
    measureTuple = std::make_tuple(hzAngle, Vangle,circle+1);
}


double TheodoliteMeasure::getHzAngle() const
{
    return std::get<0> (measureTuple);
}
double TheodoliteMeasure::getVAngle() const
{
    return std::get<1> (measureTuple);
}
qint32 TheodoliteMeasure::getCircle() const
{
    return std::get<2>(measureTuple);
}
