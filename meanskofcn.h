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
enum angleType
{
    HZ_A,
    V_A
};

double calcCircleMean(const MeasuresFromTheodolite& measureVector,const angleType& type)
{
    const double transToRad = M_PI/180;
    double EV = 0;
    double sumCos = 0;
    double sumSin = 0;

    switch (type) {
    case HZ_A:
        for (const auto&i : measureVector)
        {
            sumCos += cos(i.getHzAngle() * transToRad);
            sumSin += sin(i.getHzAngle() * transToRad);
        }
        break;
    case V_A:
        for (const auto&i : measureVector)
        {
            sumCos += cos(i.getVAngle() * transToRad);
            sumSin += sin(i.getVAngle() * transToRad);
        }
        break;
    }
    EV = atan2(sumSin,sumCos);
    return EV;
}



double calcCircleSko(const MeasuresFromTheodolite& measureVector,const double& EV,const angleType& type)
{
    const double transToRad = M_PI/180;
    QVector <double> asinsin(measureVector.size());
    double sko = 0;

    switch (type) {
    case HZ_A:
        for (int i = 0;i < asinsin.size();i++)
        {
            asinsin[i] = asin(sin(measureVector[i].getHzAngle() * transToRad));
        }
        break;
    case V_A:
        for (int i = 0;i < asinsin.size();i++)
        {
            asinsin[i] = asin(sin(measureVector[i].getVAngle() * transToRad));
        }
        break;
    }
    for (auto &i : asinsin)
    {
        double dif = pow((i - EV), 2);
        sko += dif;
    }
    qint32 countOfMeasures = measureVector.size();
    sko = sko / countOfMeasures;
    sko = sqrt(sko);
    return sko;
}

double calcMean(const MeasuresFromTheodolite& measureVector,const angleType& type)
{
    double sum=0;
    switch (type) {
    case HZ_A:
        sum = std::accumulate(measureVector.begin(), measureVector.end(), 0.0, [](auto &a, auto &b){return a + b.getHzAngle();});
        break;
    case V_A:
        sum = std::accumulate(measureVector.begin(), measureVector.end(), 0.0, [](auto &a, auto &b){return a + b.getVAngle();});
        break;
    }
    qint32 countOfMeasures = measureVector.size();
    double EV = sum / countOfMeasures;
    return EV;
}

double calcSko(const MeasuresFromTheodolite& measureVector, const double& EV, const angleType& type)
{
    double sko = 0;
    switch (type) {
    case HZ_A:

        for (auto &i : measureVector)
        {
            double dif = pow((i.getHzAngle() - EV), 2);
            sko += dif;
        }
        break;
    case V_A:

        for (auto &i : measureVector)
        {
            double dif = pow((i.getVAngle() - EV), 2);
            sko += dif;
        }
        break;
    }
    qint32 countOfMeasures = measureVector.size();
    sko = sko / countOfMeasures;
    sko = sqrt(sko);
    return sko;
}

}


QString zeroPadding(const double &number, const int &precision)
{

    QString strNumber = QString::number(number,'f',precision);

    if (number < 0)
    {
        return strNumber;
    }

    if (number < 100 && number > 9)
    {
        strNumber = "0"+strNumber;
        return strNumber;
    }
    if (number < 10)
    {
        strNumber="00" + strNumber;
        return strNumber;
    }
    return strNumber;
}

QString zeroPadding(const int &number)
{
    QString strNumber = QString::number(number);

    if (number < 0)
    {
        return strNumber;
    }

    if (number < 100 && number > 9)
    {
        strNumber = "0" + strNumber;
        return strNumber;
    }
    if (number < 10)
    {
        strNumber="00" + strNumber;
        return strNumber;
    }
    return strNumber;
}
#endif // MEANSKOFCN_H
