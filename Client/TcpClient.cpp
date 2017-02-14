#include <QSqlDatabase>
#include <QThread>
#include "TcpClient.h"
#include "Misc/Logger.h"
#include "Misc/AppSettings.h"
#include <QThread>

TcpClient::TcpClient(QTcpSocket *pSocket, QObject *pParent)
    : AClient(pParent),
      m_pSocket(pSocket)
{
    connect(m_pSocket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));

    setDataSource(m_pSocket, eTcpIp);

    //tcp client will connect immediately so we do these here, serial client we delay till COM port connects
    m_TimeOfConnect = QDateTime::currentDateTime();
}

TcpClient::~TcpClient()
{

}

void TcpClient::disconnectClient()
{
    m_pSocket->disconnectFromHost();
    m_pDataTimer->stop();
}

QString TcpClient::getClientAddress() const
{
    QString ip = "";

    if(m_pSocket != NULL) {
        ip = m_pSocket->peerAddress().toString();
    }

    return ip;
}

void TcpClient::onSocketDisconnected()
{
    emit error(QString("Client %1 at %2 disconnected").arg(m_ClientId).arg(getClientAddress()));

    //if client never got an ID, set it back to unknown state
    if(m_ClientId == "Unknown")
        m_ClientState = eUnknownState;
    else
        m_ClientState = eOffline;

    m_TimeOfDisconnect = QDateTime::currentDateTime();

    //remove the database connection for this thread
    //AppSettings settings;
    QSqlDatabase db;
//    QString dsn = QString("Driver={sql server};server=%1;database=%2;uid=%3;pwd=%4;")
//            .arg(settings.readDatabaseSettings("host", "").toString())
//            .arg(settings.readDatabaseSettings("DbName", "").toString())
//            .arg(settings.readDatabaseSettings("user", "").toString())
//            .arg(settings.readDatabaseSettings("password", "").toString());
//    db.setDatabaseName(dsn);

    QString connectionName = QString::number((int)(thread()->currentThreadId()));

    if(db.contains(connectionName)) {
        db.removeDatabase(connectionName);
    }

    //emit signal to notify model
    emit clientDataChanged();

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
