#include "gms.h"
#define _USE_MATH_DEFINES
#include <math.h>



GMS::GMS():
    gradus(0),
    minutes(0),
    seconds(0)
{

}



GMS::GMS(const double &gradAngle)
{

    gradus=gradAngle;
    minutes=std::abs(gradAngle-gradus)*60;
    seconds=(std::abs(gradAngle-gradus)-(static_cast<double>(minutes)/60))*3600;

    if(gradAngle<0 && gradAngle>-1)
    {
        minutes=-minutes;
        seconds=-seconds;
    }
}




GMS::GMS( const qint32 _gradus, const qint32 _minutes, const double _seconds)
{
    gradus=_gradus;
    minutes=_minutes;
    seconds=_seconds;
}




void GMS::setAngleValue(const double& gradAngle)
{
    gradus=gradAngle;
    minutes=std::abs(gradAngle-gradus)*60;
    seconds=(std::abs(gradAngle-gradus)-(static_cast<double>(minutes)/60))*3600;

    if(gradAngle<0 && gradAngle>-1)
    {
        minutes=-minutes;
        seconds=-seconds;
    }
}



double GMS::transToGrad()
{
    double grad;
    if(this->gradus<0)
    {
        grad=this->gradus-(static_cast<double>(this->minutes)/60)-(this->seconds/3600);
    }
    else
    {
        grad=this->gradus+(static_cast<double>(this->minutes)/60)+(this->seconds/3600);
    }

    return grad;
}



double GMS::transToRad()
{
    return transToGrad()*M_PI/180;
}



void GMS::setGradus(const qint32  _gradus)
{
    gradus=_gradus;
}


void GMS::setMinutes(const qint32  _minutes)
{
    minutes=_minutes;
}


void GMS::setSeconds(const double _seconds)
{
    seconds=_seconds;
}



bool GMS::checkStringGMSAngle(const QString& angle_string)
{
    if(angle_string.isEmpty())
    {
        return false;
    }
    QStringList split_angle_string=angle_string.split(" ");

    if(split_angle_string.size()<3)
    {
        return false;
    }
    qint32 minutes=split_angle_string.at(1).toInt();
    double seconds=split_angle_string.at(2).toDouble();

    if(minutes>60
            || minutes<0
            || seconds>60
            || seconds<0)
    {
        return false;
    }
    return true;
}



QDebug operator<< (QDebug d, const GMS &data)
{
    d << data.gradus<<data.minutes<<data.seconds;
    return d;
}



bool GMS::isEmpty() const
{
    return !gradus&&!minutes&&!seconds;
}
