#pragma once

#include <QObject>
#include <QVariant>
#include <QVector>
#include "tData.h"

class ClientData : public QObject
{
    Q_OBJECT

public:
    enum eDataId {
        eClientDate = 0,
        eTemperature,
        eHumidity,
        eNIon,
        ePIon,
        eWindDirection,
        eWindSpeed,
        eRainfall,
        ePressure,
        eUltraViolet,
        eOxygen,
        ePm1,
        ePm25,
        ePm10,
        eCO2,
        eLatitude,
        eLongtitude,
        eAltitude,
        eVOC,
        eServiceType,
        eDeviceType,
        eStationID,
        eDeviceID,
        eStatus,
        eInterval,
        eElementCount,
        eStatusCount,
        eFanOnIonCountN,
        eFanOffIonCountN,
        eFanOnIonCountP,
        eFanOffIonCountP,
        ePolarVoltP,
        ePolarVoltN,
        eTubeTempL,
        eTubeTempR,
        eTubeHumidityL,
        eTubeHumidityR,
        eInsulation,
        eRPML,
        eRPMR,
        eStatusCode,
        eQualityControl,

        eTotal
    };

    explicit ClientData(QObject *parent = 0);

    void setData(const eDataId id, const QVariant &value);

    template <class T>
    T getData(const eDataId id, const bool &canBeNull=false) const;

signals:

public slots:

private:
    QVector<tData> m_DataList;
    void validateData(eDataId id, const QVariant &value, const bool &ok);
};

