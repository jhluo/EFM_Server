#include <QThread>
#include <QSqlDatabase>
#include "TheServer.h"
#include "Client/TcpClient.h"
#include "Client/SerialClient.h"
#include "Client/AClientList.h"
#include "Misc/Logger.h"
#include "Misc/AppSettings.h"

TheServer::TheServer(QObject *pParent)
    : QTcpServer(pParent)
{
    m_pClientList = new AClientList(this);

    connect(this, SIGNAL(newConnection()), this, SLOT(onNewTcpClientConnected()));
}

TheServer::~TheServer()
{

}

void TheServer::startServer(const QString &host, const int port)
{
    QHostAddress addr;
    addr.setAddress(host);
    addr.toIPv4Address();

    if (!this->listen(addr, port)) {
        LOG_SYS(QString("Unable to start the server: %1.").arg(this->errorString()));
        close();
        return;
    } else {
        LOG_SYS(QString("Server Started, listening on %1 at port %2")
                .arg(addr.toString())
                .arg(port)
                );
    }

    //Test the database
//    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
//    QString dsn = QString("Driver={sql server};server=%1;database=%2;uid=%3;pwd=%4;")
//            .arg(settings.readDatabaseSettings("host", "").toString())
//            .arg(settings.readDatabaseSettings("DbName", "").toString())
//            .arg(settings.readDatabaseSettings("user", "").toString())
//            .arg(settings.readDatabaseSettings("password", "").toString());

//    db.setDatabaseName(dsn);
//    if(!db.open()) {
//        LOG_SYS("Failed to connect to database! Go to Settings->Database Settings to configure your database settings");
//    } else {
//        LOG_SYS("Database connection is ready.");
//    }
}

void TheServer::shutdownServer()
{
    LOG_SYS("Stopping all clients and shutting down the server...");

    //remove all clients and close the server
    m_pClientList->removeAll();

    this->close();
}

//for adding serial client
void TheServer::addSerialClient(QSerialPort *pPort)
{
    //a new client has connected, we want to create a new client object and put it into its own thread
    LOG_SYS(QString("New client from %1 has connected").arg(pPort->portName()));

    SerialClient *pClient = new SerialClient(pPort);

    QThread *pClientThread = new QThread(this);

    pClient->moveToThread(pClientThread);

    //stop the thread and clean up when pClient is disconnected
    connect(pClient, SIGNAL(clientIDAssigned()), this, SLOT(onClientIDAssigned()));
    connect(pClientThread, SIGNAL(finished()), pClientThread, SLOT(deleteLater()));

    pClientThread->start();

    m_pClientList->addClient(pClient);
}

int TheServer::getClientCount() const
{
    return m_pClientList->size();
}

//for adding tcp client
void TheServer::onNewTcpClientConnected()
{
    QTcpSocket *pSocket = this->nextPendingConnection();

    //a new client has connected, we want to create a new client object and put it into its own thread
    LOG_SYS(QString("New client from %1 at port %2 has connected").arg(pSocket->peerAddress().toString())
            .arg(pSocket->peerPort()));

    TcpClient *pClient = new TcpClient(pSocket);

    QThread *pClientThread = new QThread(this);

    pClient->moveToThread(pClientThread);

    //stop the thread and clean up when pClient is disconnected
    connect(pClient, SIGNAL(clientIDAssigned()), this, SLOT(onClientIDAssigned()));
    connect(pClient, SIGNAL(clientDisconnected()), pClientThread, SLOT(quit()));
    connect(pClientThread, SIGNAL(finished()), pClientThread, SLOT(deleteLater()));
    connect(pClient, SIGNAL(clientDisconnected()), this, SLOT(onTcpClientDisconnected()));

    pClientThread->start();

    m_pClientList->addClient(pClient);
}

void TheServer::onTcpClientDisconnected()
{
    TcpClient* pClient = static_cast<TcpClient*>(QObject::sender());
    //If this client was connected without ever giving an id, just remove it from list
    for(int i=0; i<m_pClientList->size(); i++) {
        if(m_pClientList->getClient(i)->getClientAddress() == pClient->getClientAddress()
           && m_pClientList->getClient(i)->getClientState() == AClient::eUnknownState) {
            m_pClientList->removeClient(i);
            break;
        }
    }
}

//this is needed to get rid of dead clients with the same ID
void TheServer::onClientIDAssigned()
{
    AClient* pClient = static_cast<AClient*>(QObject::sender());
    //check to see if there is already a client with the same id
    int count = 0;
    for(int i=0; i<m_pClientList->size(); i++) {
        if(m_pClientList->getClient(i)->getClientId() == pClient->getClientId()) {
            //if there was one that's previously connected but offline, remove it
            AClient* pOldClient = m_pClientList->getClient(i);
            if(pOldClient->getClientState() == AClient::eOffline) {
                //remove all dead clients in the list with same id
                //can't use ...->removeClient(i) because list with shrink as we remove
                m_pClientList->removeAClient(pOldClient);
            } else {
                //count how many identical device
                count++;
            }
        }
    }
}
