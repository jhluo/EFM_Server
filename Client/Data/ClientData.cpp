#include "ClientData.h"
#include "Misc/OffsetSettings.h"

ClientData::ClientData(QObject *parent):
    QObject(parent)
{

}

void ClientData::setData(const eDataId id, const QVariant &value)
{
    bool ok = false;
    validateData(id, value, ok);

    if(ok) {
        tData data(value);
        m_DataMap.insert(id, data);
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

    return m_DataMap.value(id).value();
}
