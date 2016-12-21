#include "SerialClient.h"
#include "Misc/Logger.h"

SerialClient::SerialClient(QSerialPort *pPort, QObject *pParent)
    : AClient(pParent),
      m_pPort(pPort)
{
    setDataSource(m_pPort, eSerial);
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
            m_pDataStarvedTimer->start();
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
    m_pDataStarvedTimer->stop();
    emit clientDataChanged();
}
