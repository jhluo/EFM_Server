#include "ClientData.h"

void ClientData::setData(const eDataId id, const QVariant &value)
{
    bool ok = false;
    validateData(id, value, ok);
    if(ok) {
        m_DataList[id].setValue(value);
    } else {
        m_DataList[id].setValue(QVariant());
    }
}

void ClientData::validateData(eDataId id, const QVariant &value, const bool &ok)
{

}

template <class T>
T ClientData::getData(const eDataId id, const bool &canBeNull) const
{

}
