#include "TcpClient.h"
#include "Misc/Logger.h"

TcpClient::TcpClient(QTcpSocket *pSocket, QObject *pParent)
    : AClient(pParent),
      m_pSocket(pSocket)
{
    connect(m_pSocket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));

    setDataSource(m_pSocket, eTcp);

    //tcp client will connect immediately so we do these here, serial client we delay till COM port connects
    m_TimeOfConnect = QDateTime::currentDateTime();
    m_pDataStarvedTimer->start();
}

TcpClient::~TcpClient()
{

}

void TcpClient::disconnectClient()
{
    m_pSocket->disconnectFromHost();
}

void TcpClient::onSocketDisconnected()
{
    LOG_SYS(QString("Client %1 at %2 disconnected").arg(m_ClientId).arg(getClientAddress()));

    if(m_pDataStarvedTimer->isActive()) {
        m_pDataStarvedTimer->stop();
    }

    m_ClientState = eOffline;

    m_TimeOfDisconnect = QDateTime::currentDateTime();

    //emit signal to notify model
    emit clientDisconnected();

    /*
    //end the thread
    this->thread()->quit();
    if(!this->thread()->wait(2000)) //Wait until it actually has terminated (max. 3 sec)
    {
        this->thread()->terminate(); //Thread didn't exit in time, probably deadlocked, terminate it!
        this->thread()->wait(); //We have to wait again here!
    }
    */
}
