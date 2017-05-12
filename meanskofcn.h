#ifndef MEANSKOFCN_H
#define MEANSKOFCN_H
#include <QVector>
#include <tuple>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>
#include <theodolite.h>


namespace meanSkoFcn
{
enum ANGLE_TYPE
{
    HZ_A,
    V_A
};

double calc_circle_mean(const MeasuresFromTheodolite& measure_vector,const ANGLE_TYPE& type)
{
    const double trans_to_rad = M_PI/180;
    double EV = 0;
    double sum_cos = 0;
    double sum_sin = 0;

    switch (type) {
    case HZ_A:
        for(auto&i:measure_vector)
        {
            sum_cos += cos(i.getHzAngle()*trans_to_rad);
            sum_sin += sin(i.getHzAngle()*trans_to_rad);
        }
        break;
    case V_A:
        for(auto&i:measure_vector)
        {
            sum_cos += cos(i.getVAngle()*trans_to_rad);
            sum_sin += sin(i.getVAngle()*trans_to_rad);
        }
        break;
    }
    EV = atan2(sum_sin,sum_cos);
    return EV;
}



double calc_circle_sko(const MeasuresFromTheodolite& measure_vector,const double& EV,const ANGLE_TYPE& type)
{
    const double trans_to_rad=M_PI/180;
    QVector <double> asinsin(measure_vector.size());
    double sko=0;

    switch (type) {
    case HZ_A:
        for(int i = 0;i < asinsin.size();i++)
        {
            asinsin[i] = asin(sin(measure_vector[i].getHzAngle()*trans_to_rad));
        }
        break;
    case V_A:
        for(int i = 0;i < asinsin.size();i++)
        {
            asinsin[i] = asin(sin(measure_vector[i].getVAngle()*trans_to_rad));
        }
        break;
    }
    for(auto &i : asinsin)
    {
        double dif=pow((i-EV),2);
        sko+=dif;
    }
    qint32 countOfMeasures = measure_vector.size();
    sko = sko/countOfMeasures;
    sko = sqrt(sko);
    return sko;
}

double calc_mean(const MeasuresFromTheodolite& measure_vector,const ANGLE_TYPE& type)
{
    double sum=0;
    switch (type) {
    case HZ_A:
        sum = std::accumulate(measure_vector.begin(),measure_vector.end(),0.0,[](auto &a,auto &b){return a+b.getHzAngle();});
        break;
    case V_A:
        sum = std::accumulate(measure_vector.begin(),measure_vector.end(),0.0,[](auto &a,auto &b){return a+b.getVAngle();});
        break;
    }
    qint32 countOfMeasures=measure_vector.size();
    double EV=sum/countOfMeasures;
    return EV;
}

double calc_sko(const MeasuresFromTheodolite& measure_vector,const double& EV, const ANGLE_TYPE& type)
{
    double sko = 0;
    switch (type) {
    case HZ_A:

        for(auto &i : measure_vector)
        {
            double dif = pow((i.getHzAngle()-EV),2);
            sko += dif;
        }
        break;
    case V_A:

        for(auto &i : measure_vector)
        {
            double dif = pow((i.getVAngle()-EV),2);
            sko += dif;
        }
        break;
    }
    qint32 countOfMeasures = measure_vector.size();
    sko = sko/countOfMeasures;
    sko = sqrt(sko);
    return sko;
}

}


QString zeroPadding(const double &number, const int &precision)
{

    QString str_number = QString::number(number,'f',precision);

    if(number < 0)
    {
        return str_number;
    }

    if(number < 100 && number > 9)
    {
        str_number = "0"+str_number;
        return str_number;
    }
    if(number < 10)
    {
        str_number="00" + str_number;
        return str_number;
    }
    return str_number;
}

QString zeroPadding(const int &number)
{
    QString str_number = QString::number(number);

    if(number < 0)
    {
        return str_number;
    }

    if(number < 100 && number > 9)
    {
        str_number = "0"+str_number;
        return str_number;
    }
    if(number < 10)
    {
        str_number="00" + str_number;
        return str_number;
    }
    return str_number;
}
#endif // MEANSKOFCN_H
