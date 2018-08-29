#include "theodolite.h"
#include <QDebug>
#include <com_pub.hpp>
#define _USE_MATH_DEFINES
#include <math.h>



Theodolite::Theodolite(const GRC_TYPE _rc, bool _cnct):
    returnedCode(_rc),connectState(_cnct)
{

}

bool Theodolite::connectLibrary(const QString &filename)
{
    geocomOrig = new QLibrary(filename);
    return geocomOrig->load();
}

qint32 Theodolite::getInstrumentNumber()
{
    if (isConnected())
    {
        long instrumentNumber = 0;
        ThdGetInstrumentNo GetInstrumentalNo = (ThdGetInstrumentNo) geocomOrig->resolve("?CSV_GetInstrumentNo@@YAFAAJ@Z");
        if (GetInstrumentalNo)
            returnedCode = GetInstrumentalNo(instrumentNumber);
        if (returnedCode == GRC_OK)
            return instrumentNumber;
    }
    return 0;
}

bool Theodolite::connect(COM_PORT port, COM_BAUD_RATE def_br,short nRetries)
{
    if (geocomOrig->isLoaded())
    {
        ThdComInit ComInit=(ThdComInit) geocomOrig->resolve("?COM_Init@@YAFXZ");
        if (ComInit)
            returnedCode = ComInit();

        if (returnedCode == GRC_OK)
        {
            ThdOpenConnection OpenConnection = (ThdOpenConnection)geocomOrig->resolve("?COM_OpenConnection@@YAFW4COM_PORT@@AAW4COM_BAUD_RATE@@F@Z");
            if (OpenConnection) returnedCode = OpenConnection(port, def_br, nRetries);

            if (returnedCode == GRC_OK)
            {
                connectState = true;
                emit connected(getLastRetMes());
                return connectState;
            }
            emit errorReport(getLastRetMes());
            return connectState;
        }
    }
    qDebug()<<"Geocom is not load";
    return connectState;

}




bool Theodolite::closeConnection()
{
    if (isConnected())
    {

        ThdCloseConnection CloseConnection = (ThdCloseConnection) geocomOrig->resolve("?COM_CloseConnection@@YAFXZ");
        if (CloseConnection)
            returnedCode = CloseConnection();
        if (returnedCode != GRC_OK)
            return false;

        ThdComEnd ComEnd =(ThdComEnd) geocomOrig->resolve("?COM_End@@YAFXZ");
        if (ComEnd)
            returnedCode = ComEnd();
        if (returnedCode != GRC_OK)
            return false;

    }
    connectState = false;
    return true;
}




QString Theodolite::getLastRetMes()
{
    char messageBuffer[255] = {0};
    QString message;
    ThdGetErrorText GetErrorText = (ThdGetErrorText) geocomOrig->resolve("?COM_GetErrorText@@YAFFPAD@Z");
    if (GetErrorText) GetErrorText(returnedCode, messageBuffer);

    for (uint i = 0; i < sizeof(messageBuffer); i++)
    {
        if ( messageBuffer[i])
        {
            message += messageBuffer[i];
        }
    }
    return message;
}




MeasuresFromTheodolite Theodolite::startMeasures(qint32 amountOfMeasures, MES_TYPE angleType)
{
    MeasuresFromTheodolite measureList(amountOfMeasures);
    if (isConnected())
    {
        TMC_ANGLE rawMeasures;
        TheodoliteMeasure thdMeasure;
        ThdGetAngle GetAngle = (ThdGetAngle) geocomOrig->resolve("?TMC_GetAngle@@YAFAAUTMC_ANGLE@@W4TMC_INCLINE_PRG@@@Z");
        if (GetAngle)
            for (auto i = 0; i < amountOfMeasures; i++)
            {
                returnedCode = GetAngle(rawMeasures, TMC_AUTO_INC);
                if (returnedCode == GRC_OK)
                {
                    if (angleType == THD_MES)
                    {
                        thdMeasure.setMeasureData(rawMeasures.dHz * transToRad,
                                                   rawMeasures.dV * transToRad,
                                                   rawMeasures.eFace);
                        measureList[i] = thdMeasure;
                    }
                    else if (angleType == THD_INCL)
                    {
                        thdMeasure.setMeasureData(rawMeasures.Incline.dCrossIncline * transToRad,
                                                   rawMeasures.Incline.dLengthIncline * transToRad,
                                                   rawMeasures.eFace);
                        measureList[i] = thdMeasure;
                    }

                }
                else
                {
                    errorReport(getLastRetMes());
                    continue;
                }
            }
    }
    return measureList;
}


TheodoliteMeasure Theodolite::makeOneMeasure(MES_TYPE angleType)
{
    TheodoliteMeasure thdMeasure;
    if (isConnected())
    {
        TMC_ANGLE rawMeasures;
        ThdGetAngle GetAngle = (ThdGetAngle) geocomOrig->resolve("?TMC_GetAngle@@YAFAAUTMC_ANGLE@@W4TMC_INCLINE_PRG@@@Z");
        if (GetAngle)
            returnedCode = GetAngle(rawMeasures, TMC_AUTO_INC);

        if (returnedCode == GRC_OK)
        {
            if (angleType == THD_MES)
            {
                thdMeasure.setMeasureData(rawMeasures.dHz * transToRad,
                                           rawMeasures.dV * transToRad,
                                           rawMeasures.eFace);
            }
            else if (angleType==THD_INCL)
            {
                thdMeasure.setMeasureData(rawMeasures.Incline.dCrossIncline * transToRad,
                                           rawMeasures.Incline.dLengthIncline * transToRad,
                                           rawMeasures.eFace);
            }
        }

        errorReport(getLastRetMes());
    }
    return thdMeasure;
}


bool Theodolite::resetHz(const double& HzOrient)
{
    if (isConnected())
    {
        ThdDoMeasure doMeasure = (ThdDoMeasure) geocomOrig->resolve("?TMC_DoMeasure@@YAFW4TMC_MEASURE_PRG@@@Z");
        if (doMeasure)
            returnedCode = doMeasure(TMC_CLEAR);
        if (returnedCode != GRC_OK)
        {
            errorReport(getLastRetMes());
            return false;
        }
        ThdSetOrientation SetOrientationHz = (ThdSetOrientation) geocomOrig->resolve("?TMC_SetOrientation@@YAFN@Z");
        if (SetOrientationHz)
            returnedCode = SetOrientationHz(HzOrient);
        if (returnedCode != GRC_OK)
        {
            errorReport(getLastRetMes());
            return false;
        }
        return true;
    }
    return false;
}

bool Theodolite::checkCompensator(const double& inclinationLimit)
{
    TheodoliteMeasure compensatorAngles;
    compensatorAngles = makeOneMeasure(THD_INCL);
    double firstHzInSec = compensatorAngles.getHzAngle() * 3600;
    double secondHzInSec = compensatorAngles.getVAngle() * 3600;
    return std::fabs(firstHzInSec) < inclinationLimit
            && std::fabs(secondHzInSec) < inclinationLimit;

}


Theodolite::~Theodolite()
{

    closeConnection();
    delete geocomOrig;
}
