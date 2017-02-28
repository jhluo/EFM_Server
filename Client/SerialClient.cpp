#include "SerialClient.h"
#include "Misc/Logger.h"

SerialClient::SerialClient(QSerialPort *pPort, QObject *pParent)
    : AClient(pPort, pParent)
{
    //setDataSource(m_pPort, eSerial);
}

SerialClient::~SerialClient()
{

}

void SerialClient::connectClient()
{
    if(m_pPort->open(QIODevice::ReadWrite)) {
            m_ClientState = eNoData;
            m_TimeOfConnect = QDateTime::currentDateTime();
            m_TimeOfDisconnect = QDateTime();
            m_pDataTimer->start();
            emit clientDataChanged();
    } else {
        emit error(m_pPort->errorString());
    }
}

void SerialClient::disconnectClient()
{
    m_pPort->close();
    m_ClientState = eOffline;
    m_TimeOfDisconnect = QDateTime::currentDateTime();
    m_pDataTimer->stop();
    emit clientDataChanged();
}

QString SerialClient::getClientAddress() const
{
    QString port = "";

    if(m_pPort != NULL) {
        port = m_pPort->portName();
    }

    return port;
}

QIODevice* SerialClient::getInputDevice()
{
    return m_pPort;
}
