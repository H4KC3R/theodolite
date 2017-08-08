#include "theodolite.h"
#include <QDebug>
#include <com_pub.hpp>
#define _USE_MATH_DEFINES
#include <math.h>



Theodolite::Theodolite(const GRC_TYPE _rc, bool _cnct):
    returnedCode(_rc),connect_state(_cnct)
{

}

bool Theodolite::connectLibrary(const QString &filename)
{
    geocom_orig = new QLibrary(filename);
    return geocom_orig->load();
}

quint32 Theodolite::getInstrumentNumber()
{
    if (isConnected())
    {
        long instrument_number = 0;
        ThdGetInstrumentNo GetInstrumentalNo = (ThdGetInstrumentNo) geocom_orig->resolve("?CSV_GetInstrumentNo@@YAFAAJ@Z");
        if (GetInstrumentalNo) returnedCode = GetInstrumentalNo(instrument_number);
        if (returnedCode == GRC_OK) return instrument_number;

    }
    return 0;
}

bool Theodolite::enableConnection(COM_PORT port, COM_BAUD_RATE def_br,short nRetries)
{
    if (geocom_orig->isLoaded())
    {
        ThdComInit ComInit=(ThdComInit) geocom_orig->resolve("?COM_Init@@YAFXZ");
        if (ComInit) returnedCode = ComInit();

        if (returnedCode == GRC_OK)
        {
            ThdOpenConnection OpenConnection = (ThdOpenConnection)geocom_orig->resolve("?COM_OpenConnection@@YAFW4COM_PORT@@AAW4COM_BAUD_RATE@@F@Z");
            if (OpenConnection) returnedCode = OpenConnection(port, def_br, nRetries);

            if (returnedCode == GRC_OK)
            {
                connect_state = true;
                emit connected(getLastRetMes());
                return connect_state;
            }
            emit error_report(getLastRetMes());
            return connect_state;
        }
    }
    qDebug()<<"Geocom is not load";
    return connect_state;

}




bool Theodolite::closeConnection()
{
    if (isConnected())
    {

        ThdCloseConnection CloseConnection = (ThdCloseConnection) geocom_orig->resolve("?COM_CloseConnection@@YAFXZ");
        if (CloseConnection) returnedCode = CloseConnection();
        if (returnedCode != GRC_OK) return false;

        ThdComEnd ComEnd =(ThdComEnd) geocom_orig->resolve("?COM_End@@YAFXZ");
        if (ComEnd) returnedCode = ComEnd();
        if (returnedCode != GRC_OK) return false;

    }
    connect_state = false;
    return true;
}




QString Theodolite::getLastRetMes()
{
    char message_buffer[255] = {0};
    QString message;
    ThdGetErrorText GetErrorText = (ThdGetErrorText) geocom_orig->resolve("?COM_GetErrorText@@YAFFPAD@Z");
    if (GetErrorText) GetErrorText(returnedCode, message_buffer);

    for (int i = 0;i < sizeof(message_buffer); i ++)
    {
        if ( message_buffer[i])
        {
            message += message_buffer[i];
        }
    }
    return message;
}




MeasuresFromTheodolite Theodolite::startMeasures(quint16 amount_of_measures, MES_TYPE angle_type)
{
    MeasuresFromTheodolite measure_list(amount_of_measures);
    if (isConnected())
    {
        const constexpr double trans_to_rad=180/M_PI;
        TMC_ANGLE raw_measures;
        TheodoliteMeasure thd_measure;
        ThdGetAngle GetAngle = (ThdGetAngle) geocom_orig->resolve("?TMC_GetAngle@@YAFAAUTMC_ANGLE@@W4TMC_INCLINE_PRG@@@Z");
        if (GetAngle)
            for (auto i = 0;i < amount_of_measures;i++)
            {
                returnedCode = GetAngle(raw_measures, TMC_AUTO_INC);
                if (returnedCode == GRC_OK)
                {
                    if (angle_type == THD_MES)
                    {
                        thd_measure.setMeasureData(raw_measures.dHz*trans_to_rad,raw_measures.dV*trans_to_rad,raw_measures.eFace);
                        measure_list[i] = thd_measure;
                    }
                    else if (angle_type == THD_INCL)
                    {
                        thd_measure.setMeasureData(raw_measures.Incline.dCrossIncline*trans_to_rad,raw_measures.Incline.dLengthIncline*trans_to_rad,raw_measures.eFace);
                        measure_list[i] = thd_measure;
                    }

                }
                else
                {
                    error_report(getLastRetMes());
                    continue;
                }
            }
    }
    return measure_list;
}


TheodoliteMeasure Theodolite::makeOneMeasure(MES_TYPE angle_type)
{
    TheodoliteMeasure thd_measure;
    if (isConnected())
    {
        TMC_ANGLE raw_measures;
        const constexpr double trans_to_rad = 180/M_PI;

        ThdGetAngle GetAngle = (ThdGetAngle) geocom_orig->resolve("?TMC_GetAngle@@YAFAAUTMC_ANGLE@@W4TMC_INCLINE_PRG@@@Z");
        if (GetAngle)
            returnedCode = GetAngle(raw_measures, TMC_AUTO_INC);

        if (returnedCode == GRC_OK)
        {
            if (angle_type == THD_MES)
            {
                thd_measure.setMeasureData(raw_measures.dHz*trans_to_rad,raw_measures.dV*trans_to_rad,raw_measures.eFace);
            }
            else if (angle_type==THD_INCL)
            {
                thd_measure.setMeasureData(raw_measures.Incline.dCrossIncline*trans_to_rad,raw_measures.Incline.dLengthIncline*trans_to_rad,raw_measures.eFace);
            }
        }

        error_report(getLastRetMes());
    }
    return thd_measure;
}


bool Theodolite::resetHz(const double& HzOrient)
{
    if (isConnected())
    {
        ThdDoMeasure DoMeasure = (ThdDoMeasure) geocom_orig->resolve("?TMC_DoMeasure@@YAFW4TMC_MEASURE_PRG@@@Z");
        if (DoMeasure) returnedCode = DoMeasure(TMC_CLEAR);
        if (returnedCode != GRC_OK)
        {
            error_report(getLastRetMes());
            return false;
        }
        ThdSetOrientation SetOrientationHz=(ThdSetOrientation) geocom_orig->resolve("?TMC_SetOrientation@@YAFN@Z");
        if (SetOrientationHz) returnedCode = SetOrientationHz(HzOrient);
        if (returnedCode != GRC_OK)
        {
            error_report(getLastRetMes());
            return false;
        }
        return true;
    }
    return false;
}

bool Theodolite::checkCompensator(const double& inclination_limit)
{
    TheodoliteMeasure compensator_angles;
    compensator_angles = makeOneMeasure(THD_INCL);
    double firstHzInSec = compensator_angles.getHzAngle() * 3600;
    double secondHzInSec = compensator_angles.getVAngle() * 3600;
    return std::abs(firstHzInSec) < inclination_limit && std::abs(secondHzInSec) < inclination_limit;

}


Theodolite::~Theodolite()
{

    closeConnection();
    delete geocom_orig;
}
