#include "tData.h"

#define DATA_TIMEOUT 60000 * 2

tData::tData(QObject *pParent) :
    QObject(pParent)
{
    m_DataValue = QVariant();

    //time out the data
    m_pDataTimer = new QTimer(this);
    m_pDataTimer->setInterval(DATA_TIMEOUT);
    connect(m_pDataTimer, SIGNAL(timeout()), this, SLOT(onDataTimeout()));
}

tData::tData(const QVariant &value, QObject *pParent) :
    tData(pParent)
{
    m_DataValue = value;
}

//copy constructor
tData::tData(const tData &data) :
    tData(data.parent())
{
    this->setValue(data.value());
}

tData::~tData()
{
    //m_pDataTimer->stop();
}

void tData::setValue(const QVariant &value)
{
    m_DataValue = value;

    if (value.isValid()) {
        m_pDataTimer->stop();
        m_pDataTimer->start();
    }
}

void tData::onDataTimeout()
{
    m_DataValue = QVariant();
}
