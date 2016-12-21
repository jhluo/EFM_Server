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

void TheServer::startServer()
{
    /*hardcoded to localhost and port 5101, this can be made more flexible
        by creating a server settings dialog */
    AppSettings settings;
    QHostAddress addr;
    addr.setAddress(settings.readServerSettings("host", "127.0.0.1").toString());
    addr.toIPv4Address();
    quint16 port = settings.readServerSettings("port", "5101").toInt();

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
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    QString dsn = QString("Driver={sql server};server=%1;database=%2;uid=%3;pwd=%4;")
            .arg(settings.readDatabaseSettings("host", "").toString())
            .arg(settings.readDatabaseSettings("DbName", "").toString())
            .arg(settings.readDatabaseSettings("user", "").toString())
            .arg(settings.readDatabaseSettings("password", "").toString());

    db.setDatabaseName(dsn);
    if(!db.open()) {
        LOG_SYS("Failed to connect to database! Go to Settings->Database Settings to configure your database settings");
    } else {
        LOG_SYS("Database connection is ready.");
    }
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

    //put the socket(connection) into the client object
    pClient->setDataSource(pPort, AClient::eSerial);

    QThread *pClientThread = new QThread(this);

    pClient->moveToThread(pClientThread);

    //stop the thread and clean up when pClient is disconnected
    connect(pClient, SIGNAL(clientIDAssigned()), this, SLOT(onClientIDAssigned()));
    connect(pClientThread, SIGNAL(finished()), pClientThread, SLOT(deleteLater()));

    pClientThread->start();

    m_pClientList->addClient(pClient);
}

//for adding tcp client
void TheServer::onNewTcpClientConnected()
{
    QTcpSocket *pSocket = this->nextPendingConnection();

    //a new client has connected, we want to create a new client object and put it into its own thread
    LOG_SYS(QString("New client from %1 has connected").arg(pSocket->peerAddress().toString()));

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
        if(pClient->getClientId() == "Unknown"
           && m_pClientList->getClient(i)->getClientId() == pClient->getClientId()
           && m_pClientList->getClient(i)->getClientAddress() == pClient->getClientAddress()
           && m_pClientList->getClient(i)->getClientState() == "Offline") {
            AClient* pOldClient = m_pClientList->getClient(i);
            delete pOldClient;
            m_pClientList->removeClient(i);
        }
    }
}

void TheServer::onClientIDAssigned()
{
    AClient* pClient = static_cast<AClient*>(QObject::sender());
    //check to see if there is already a client with the same id
    int count = 0;
    for(int i=0; i<m_pClientList->size(); i++) {
        if(m_pClientList->getClient(i)->getClientId() == pClient->getClientId()) {
            //if there was one that's previously connected but offline, remove it
            AClient* pOldClient = m_pClientList->getClient(i);
            if(pOldClient->getClientState() == "Offline") {
                //delete pOldClient->thread();
                delete pOldClient;
                m_pClientList->removeClient(i);
            } else {
                //count how many identical device
                count++;
            }
        }
    }
}
