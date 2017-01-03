#include "tData.h"

#define DATA_TIMEOUT 60000 * 2

tData::tData(QObject *pParent) :
    QObject(pParent)
{
    m_DataValue = QVariant();

    //time out the data
    m_DataTimer.setInterval(DATA_TIMEOUT);
    connect(&m_DataTimer, SIGNAL(timeout()), this, SLOT(onDataTimeout()));
}

tData::tData(const tData &data) :
    QObject(data.parent())
{
    m_DataValue.setValue(data.value());
    m_DataTimer.setInterval(DATA_TIMEOUT);
    connect(&m_DataTimer, SIGNAL(timeout()), this, SLOT(onDataTimeout()));
    //m_DataTimer.stop();
}

tData::~tData()
{
    //m_DataTimer.stop();
}

void tData::setValue(const QVariant &value)
{
    m_DataValue = value;

//    if (value.isValid()) {
//        m_DataTimer.stop();
//        m_DataTimer.start();
//    }
}

void tData::onDataTimeout()
{
    m_DataValue = QVariant();
}
