﻿#include "ClientData.h"
#include "Misc/OffsetSettings.h"
#include <algorithm>

ClientData::ClientData(QObject *parent):
    QObject(parent)
{

}

void ClientData::setData(const eDataId id, const QVariant &value)
{
    bool ok = false;
    validateData(id, value, ok);

    if(ok) {
        tData *pData = m_DataMap.value(id);
        if(pData == NULL) {
            pData = new tData(value, this);
            m_DataMap.insert(id, pData);
        } else {
            pData->setValue(value);
        }
    }
}


void ClientData::validateData(const eDataId id, const QVariant &value, bool &ok)
{
    switch(id) {
    case eClientDate:
        if(!value.toString().isEmpty())
            ok = true;
    break;

    case eTemperature:
        if(value.toDouble() > -500 && value.toDouble() < 6000)
            ok = true;
    break;

    case eHumidity:
        if(value.toDouble() > -1)
            ok = true;
    break;

    case eNIon:
        if(value.toInt()<25000 && value.toInt()>5)
            ok = true;
    break;

    case ePIon:
        if(value.toInt()<25000 && value.toInt()>5)
            ok = true;
    break;

    default:
        //set it for now until all IDs are checked
        ok = true;
    break;
    }
}

QVariant ClientData::getData(const eDataId &id) const
{
//    if(m_DataMap[id].value().isNull() || !m_DataMap[id].value().isValid()) {
//        if(m_DataMap[id].value().type()==QVariant::Int)
//            return QVariant(QVariant::Int);
//        if(m_DataMap[id].value().type()==QVariant::Double)
//            return QVariant(QVariant::Double);
//        if(m_DataMap[id].value().type()==QVariant::String)
//            return QVariant(QVariant::String);
//    }
    if(m_DataMap.value(id)==NULL)
        return QVariant();
    else return m_DataMap.value(id)->value();
}

void ClientData::removeData(const eDataId id)
{
    m_DataMap.remove(id);
}

void ClientData::clear()
{
    if(m_DataMap.size() > 0) {
        qDeleteAll(m_DataMap.begin(), m_DataMap.end());
        m_DataMap.clear();
    }
}
