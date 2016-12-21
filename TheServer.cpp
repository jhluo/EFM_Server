#include <QThread>
#include <QSqlDatabase>
#include "TheServer.h"
#include "AClient.h"
#include "QTcpSocket"
#include "Misc/Logger.h"
#include "Misc/AppSettings.h"

TheServer::TheServer(QObject *pParent)
    : QTcpServer(pParent)
{
    m_pClientList = new AClientList(this);
    connect(this, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
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

void TheServer::addSerialClient(QSerialPort *pPort)
{
    //a new client has connected, we want to create a new client object and put it into its own thread
    LOG_SYS(QString("New client from %1 has connected").arg(pPort->portName()));

    AClient *pClient = new AClient;

    //put the socket(connection) into the client object
    pClient->setInputDevice(pPort, AClient::eSerial);

    QThread *pClientThread = new QThread(this);

    pClient->moveToThread(pClientThread);

    //stop the thread and clean up when pClient is disconnected
    connect(pClient, SIGNAL(clientIDAssigned()), this, SLOT(onClientIDAssigned()));
    connect(pClientThread, SIGNAL(finished()), pClientThread, SLOT(deleteLater()));

    pClientThread->start();

    //we add the serial client as they are created, different than the tcpip client
    m_pClientList->addClient(pClient);
}

void TheServer::onNewConnection()
{
    QTcpSocket *pSocket = this->nextPendingConnection();

    //a new client has connected, we want to create a new client object and put it into its own thread
    LOG_SYS(QString("New client from %1 has connected").arg(pSocket->peerAddress().toString()));

    AClient *pClient = new AClient;

    //put the socket(connection) into the client object
    pClient->setInputDevice(pSocket, AClient::eTcp);

    m_pClientList->addClient(pClient);

    QThread *pClientThread = new QThread(this);

    pClient->moveToThread(pClientThread);

    //stop the thread and clean up when pClient is disconnected
    connect(pClient, SIGNAL(clientIDAssigned()), this, SLOT(onClientIDAssigned()));
    connect(pClientThread, SIGNAL(finished()), pClientThread, SLOT(deleteLater()));

    pClientThread->start();
}


void TheServer::onClientIDAssigned()
{
    AClient* pClient = static_cast<AClient*>(QObject::sender());
    //check to see if this device was previously connected and was in "offline" state
    for(int i=0; i<m_pClientList->size(); i++) {
        if(m_pClientList->getClient(i)->getClientId() == pClient->getClientId()
           && m_pClientList->getClient(i)->getClientState() != "Online") {
            AClient* pOldClient = m_pClientList->getClient(i);
            delete pOldClient->thread();
            delete pOldClient;
            m_pClientList->removeClient(i);
        }
    }
}
