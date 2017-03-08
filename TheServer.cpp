#include <QThread>
#include <QPointer>
#include <QSqlDatabase>
#include "TheServer.h"
#include "Client/TcpClient.h"
#include "Client/SerialClient.h"
//#include "Client/AClientList.h"
#include "Client/AClient.h"
#include "Misc/Logger.h"
#include "Misc/AppSettings.h"

TheServer::TheServer(QObject *pParent)
    : QTcpServer(pParent)
{
    m_ClientHash.clear();
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

    //signals all clients to d/c
    emit serverShutdown();

    this->close();
}

//for adding tcp client
void TheServer::onNewTcpClientConnected()
{
    QPointer<QTcpSocket> pSocket = this->nextPendingConnection();

    //a new client has connected, we want to create a new client object and put it into its own thread
    LOG_SYS(QString("New client from %1 at port %2 has connected.  Waiting for data...").arg(pSocket->peerAddress().toString())
            .arg(pSocket->peerPort()));

    QPointer<TcpClient> pClient = new TcpClient(pSocket);
    addClient(pClient);

    //The client is not added to the hash table until an ID is assigned
}

void TheServer::addClient(AClient *pClient)
{
    //needs to create thread first then create database in order to get correct thread ID
    QPointer<QThread> pClientThread = new QThread();
    pClient->moveToThread(pClientThread);

    //stop the thread and clean up when pClient is disconnected
    connect(pClient, SIGNAL(clientIDAssigned()), this, SLOT(onClientIDAssigned()), Qt::QueuedConnection);
    connect(pClientThread, SIGNAL(finished()), pClient, SIGNAL(clientDisconnected()), Qt::QueuedConnection);
    connect(pClient, SIGNAL(clientDisconnected()), this, SLOT(onClientDisconnected()), Qt::QueuedConnection);
    //connect(pClientThread, SIGNAL(finished()), pClientThread, SLOT(deleteLater()));
    connect(this, SIGNAL(serverShutdown()), pClient, SLOT(onServerShutdown()), Qt::QueuedConnection);


    pClientThread->start();
}

//for adding serial client
void TheServer::addSerialClient(QSerialPort *pPort)
{
    //a new client has connected, we want to create a new client object and put it into its own thread
    LOG_SYS(QString("New client from %1 has connected").arg(pPort->portName()));

    SerialClient *pClient = new SerialClient(pPort);

    addClient(pClient);
}

//this may not be needed because we are using guarded pointers
void TheServer::removeClient(AClient *pClient)
{
    QPointer<QThread> pThread = pClient->thread();
    pClient->deleteLater();
    pThread->deleteLater();
}

//void TheServer::sortClients(const int column, Qt::SortOrder order)
//{
//    qDebug() << "Before sort:\n";
//    for(int i=0; i<m_ClientList.size(); i++) {
//        qDebug() << m_ClientList.at(i)->getClientId() << endl;
//    }
    //QMutexLocker locker(&mMutex);

//    if(column == 0) {   //sort by name
//        if(order == Qt::AscendingOrder)
//            std::sort(m_ClientHash.begin(), m_ClientHash.end(),
//                      [](AClient* a, AClient* b) -> bool { return a->getClientId().toInt() < b->getClientId().toInt(); });
//        else
//            std::sort(m_ClientHash.begin(), m_ClientHash.end(),
//                      [](AClient* a, AClient* b) -> bool { return a->getClientId().toInt() > b->getClientId().toInt(); });
//    } else if (column == 1) { //sort by source
//        if(order == Qt::AscendingOrder)
//            std::sort(m_ClientHash.begin(), m_ClientHash.end(),
//                      [](AClient* a, AClient* b) -> bool { return a->getClientAddress() < b->getClientAddress(); });
//        else
//            std::sort(m_ClientHash.begin(), m_ClientHash.end(),
//                      [](AClient* a, AClient* b) -> bool { return a->getClientAddress() > b->getClientAddress(); });
//    }

//    qDebug() << "After sort:\n";
//    for(int i=0; i<m_ClientList.size(); i++) {
//        qDebug() << m_ClientList.at(i)->getClientId() << endl;
//    }
//}

void TheServer::onClientDisconnected()
{
    TcpClient* pClient = static_cast<TcpClient*>(QObject::sender());

    Q_ASSERT(pClient!=NULL);

    //If this client was connected without ever giving an id, just remove it from list
    if(pClient->getClientState() == AClient::eUnknownState) {
        //no need to remove from hash because an unknown client is never added there
        removeClient(pClient);
    }
}

//this is needed to get rid of dead clients with the same ID
void TheServer::onClientIDAssigned()
{
    AClient* pClient = static_cast<AClient*>(QObject::sender());

    QString key = pClient->getClientId();
    if(m_ClientHash.contains(key)) {
        QPointer<AClient> pOldClient = m_ClientHash.take(key);
        //if the client with the same id is still connected somehow, d/c it
        if(pOldClient->getClientState()!=AClient::eOffline)
            QMetaObject::invokeMethod(pOldClient, "disconnectClient", Qt::QueuedConnection);
        else
            removeClient(pOldClient);
        //REVIEW: the logic above needs review

        emit clientRemoved(key);
    }
    m_ClientHash.insert(key, pClient);
    emit clientAdded(key);
}

int TheServer::getClientCount() const
{
    return m_ClientHash.size();
}

QStringList TheServer::getClientList() const
{
    return m_ClientHash.keys();
}

AClient *TheServer::getClient(const QString &key) const
{
    return m_ClientHash.value(key);
}
