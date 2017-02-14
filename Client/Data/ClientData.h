#pragma once

#include <QObject>
#include <QVariant>
#include <QMap>
#include "tData.h"

class ClientData : public QObject
{
    Q_OBJECT

public:
    enum eDataId {
        eClientId = 0,
        eClientDate,
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
        eError,

        eTotal
    };

    explicit ClientData(QObject *parent = 0);

    void setData(const eDataId id, const QVariant &value);
    QVariant getData(const eDataId &id) const;

signals:

public slots:

private:
    QMap<eDataId, tData> m_DataMap;
    void validateData(const eDataId id, const QVariant &value, bool &ok);
};

