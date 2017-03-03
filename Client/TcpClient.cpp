#include <QSqlDatabase>
#include <QThread>
#include "TcpClient.h"
#include "Misc/Logger.h"
#include "Misc/AppSettings.h"
#include <QThread>

TcpClient::TcpClient(QTcpSocket *pSocket, QObject *pParent)
    : AClient(pSocket, pParent)
      //m_pSocket(pSocket)
{
    connect(pSocket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));

    //setDataSource(m_pSocket, eTcpIp);
    m_ClientType = eTcpIp;

    //tcp client will connect immediately so we do these here, serial client we delay till COM port connects
    m_TimeOfConnect = QDateTime::currentDateTime();
}

TcpClient::~TcpClient()
{

}

void TcpClient::disconnectClient()
{
    QTcpSocket *pSocket = qobject_cast<QTcpSocket*>(m_pInputDevice);
    pSocket->disconnectFromHost();
    pSocket->close();
    pSocket=NULL;
//    if(m_pSocket==NULL) return;

//    if(m_pSocket->isOpen())
//        m_pSocket->disconnectFromHost();

//    //if client never got an ID, set it back to unknown state
//    if(m_ClientId == "Unknown")
//        m_ClientState = eUnknownState;
//    else
//        m_ClientState = eOffline;

//    m_TimeOfDisconnect = QDateTime::currentDateTime();

//    m_pDataTimer->stop();

//    m_pClientData->clear();

//    //remove database connection
//    //if(m_Database.isOpen())
//    //    m_Database.close();
//    if(!m_DbConnectionName.isEmpty())
//        QSqlDatabase::removeDatabase(m_DbConnectionName);

//    //end the thread
//    this->thread()->quit();
}

QString TcpClient::getClientAddress() const
{
    QString ip = "";
    QTcpSocket *pSocket = qobject_cast<QTcpSocket*>(m_pInputDevice);
    if(pSocket != NULL) {
        ip = pSocket->peerAddress().toString() +
                ":" + QString::number(pSocket->peerPort());
    }

    return ip;
}

void TcpClient::onSocketDisconnected()
{
    emit error(QString("Client %1 at %2 disconnected").arg(m_ClientId).arg(getClientAddress()));

    //disconnectClient();
    //if client never got an ID, set it back to unknown state
    if(m_ClientId == "Unknown")
        m_ClientState = eUnknownState;
    else
        m_ClientState = eOffline;

    m_TimeOfDisconnect = QDateTime::currentDateTime();

    m_pDataTimer->stop();

    m_pClientData->clear();

    //remove database connection
    //if(m_Database.isOpen())
    //    m_Database.close();
    if(!m_DbConnectionName.isEmpty())
        QSqlDatabase::removeDatabase(m_DbConnectionName);

    //emit signal to notify model
    emit clientDataChanged();

    //end the thread
    this->thread()->quit();

    //emit signal to server
    //emit clientDisconnected();

    /*
    if(!this->thread()->wait(2000)) //Wait until it actually has terminated (max. 3 sec)
    {
        this->thread()->terminate(); //Thread didn't exit in time, probably deadlocked, terminate it!
        this->thread()->wait(); //We have to wait again here!
    }
    */
}
