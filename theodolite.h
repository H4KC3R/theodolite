#ifndef THEODOLITE_H
#define THEODOLITE_H
#include <QObject>
#include <com_pub.hpp>
#include<QLibrary>
#include <gms.h>
#include <theodolitemeasure.h>
#include "math.h"

/*Суть в том, что библиотека для работы с теодолитами блочит следующие после первого подключения. Поэтому, данная программа использует
две копии dll, своя для каждого теодолита. Поэтому dll подключяется не целиком, а из нее лишь цепляются нужные функции.
Функция connectLibrary как раз подключает заданную копию dll*/
enum MES_TYPE
{
    THD_MES,
    THD_INCL
};

typedef QVector<TheodoliteMeasure> MeasuresFromTheodolite;

class Theodolite:public QObject
{
    Q_OBJECT
    typedef GRC_TYPE (*ThdComInit)(void);
    typedef GRC_TYPE (*ThdOpenConnection)(COM_PORT ePort,
                                         COM_BAUD_RATE& BaudRate,
                                         short nRetries);
    typedef GRC_TYPE (*ThdComEnd)();
    typedef GRC_TYPE (*ThdCloseConnection)();
    typedef GRC_TYPE (*ThdGetErrorText)(GRC_TYPE RetCode,
                                      char* szErrText);
    typedef GRC_TYPE (*ThdGetAngle)(TMC_ANGLE& Angle,
                                   TMC_INCLINE_PRG eMode);

    typedef GRC_TYPE (*ThdDoMeasure) (TMC_MEASURE_PRG Command);
    typedef GRC_TYPE (*ThdSetOrientation) (double HzOrient);
    typedef GRC_TYPE (*ThdGetInstrumentNo)(long& SerialNo);
public:


    explicit Theodolite(const GRC_TYPE rc = GRC_UNDEFINED, bool cnct = false);
    Theodolite(const Theodolite&)                   = delete;
    Theodolite(Theodolite&&)                        = delete;
    Theodolite& operator=(const Theodolite&)        = delete;
    Theodolite& operator=(Theodolite&&)             = delete;


    bool connectLibrary(const QString& );
    bool connect(COM_PORT port, COM_BAUD_RATE def_br = COM_BAUD_9600, short nRetries = 4);
    bool closeConnection();
    QString getLastRetMes();
    MeasuresFromTheodolite startMeasures(qint32, MES_TYPE);
    TheodoliteMeasure makeOneMeasure(MES_TYPE);
    bool resetHz(const double&);
    bool isConnected() const {return connectState;}
    bool checkCompensator(const double&);
    qint32 getInstrumentNumber();
    ~Theodolite();

signals:
    void connected(const QString& message);
    void errorReport(const QString& er_message);


private:
    static const constexpr double transToRad = 180 / M_PI;
    GRC_TYPE returnedCode;
    bool connectState;
    bool inProgress;
    QLibrary* geocomOrig;

};





#endif // THEODOLITE_H
