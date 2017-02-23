#include <QThread>
#include <QSqlDatabase>
#include "TheServer.h"
#include "Client/TcpClient.h"
#include "Client/SerialClient.h"
#include "Client/AClient.h"
#include "Misc/Logger.h"
#include "Misc/AppSettings.h"

TheServer::TheServer(QObject *pParent)
    : QTcpServer(pParent)
{
    m_ClientList.clear();

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
    //m_pClientList->removeAll();
    emit serverShutdown();
    this->close();
}

void TheServer::addClient(AClient *pClient)
{
    //needs to create thread first then create database in order to get correct thread ID
    QThread *pClientThread = new QThread();
    pClient->moveToThread(pClientThread);

    //stop the thread and clean up when pClient is disconnected
    connect(pClient, SIGNAL(clientIDAssigned()), this, SLOT(onClientIDAssigned()));
    //connect(pClientThread, SIGNAL(finished()), pClientThread, SLOT(deleteLater()));
    connect(pClient, SIGNAL(clientDisconnected()), this, SLOT(onClientDisconnected()));

    pClientThread->start();
    m_ClientList.append(pClient);
    emit clientAdded();
}

//for adding serial client
void TheServer::addSerialClient(QSerialPort *pPort)
{
    //a new client has connected, we want to create a new client object and put it into its own thread
    LOG_SYS(QString("New client from %1 has connected").arg(pPort->portName()));

    SerialClient *pClient = new SerialClient(pPort);

    addClient(pClient);
}

void TheServer::removeClient(AClient *pClient)
{
    //If this client was connected without ever giving an id, just remove it from list
    int index = m_ClientList.indexOf(pClient);

    if(index != -1) {
        AClient* pClient = m_ClientList[index];
        //remove client will delete the AClient object
        QThread *pThread = pClient->thread();
        delete m_ClientList[index];
        m_ClientList.remove(index);
        emit clientRemoved(index);
        pThread->deleteLater();
    }
}

void TheServer::sortClients(const int column, Qt::SortOrder order)
{
//    qDebug() << "Before sort:\n";
//    for(int i=0; i<m_ClientList.size(); i++) {
//        qDebug() << m_ClientList.at(i)->getClientId() << endl;
//    }
    //QMutexLocker locker(&mMutex);

    if(column == 0) {   //sort by name
        if(order == Qt::AscendingOrder)
            std::sort(m_ClientList.begin(), m_ClientList.end(),
                      [](AClient* a, AClient* b) -> bool { return a->getClientId().toInt() < b->getClientId().toInt(); });
        else
            std::sort(m_ClientList.begin(), m_ClientList.end(),
                      [](AClient* a, AClient* b) -> bool { return a->getClientId().toInt() > b->getClientId().toInt(); });
    } else if (column == 1) { //sort by source
        if(order == Qt::AscendingOrder)
            std::sort(m_ClientList.begin(), m_ClientList.end(),
                      [](AClient* a, AClient* b) -> bool { return a->getClientAddress() < b->getClientAddress(); });
        else
            std::sort(m_ClientList.begin(), m_ClientList.end(),
                      [](AClient* a, AClient* b) -> bool { return a->getClientAddress() > b->getClientAddress(); });
    }

//    qDebug() << "After sort:\n";
//    for(int i=0; i<m_ClientList.size(); i++) {
//        qDebug() << m_ClientList.at(i)->getClientId() << endl;
//    }
}

int TheServer::getClientCount() const
{
    return m_ClientList.size();
}

AClient* TheServer::getClient(const int index) const
{
    return m_ClientList.at(index);
}

//for adding tcp client
void TheServer::onNewTcpClientConnected()
{
    QTcpSocket *pSocket = this->nextPendingConnection();
    connect(pSocket, SIGNAL(readyRead()), this, SLOT(onIoReadyRead()));

    //a new client has connected, we want to create a new client object and put it into its own thread
    LOG_SYS(QString("New client from %1 at port %2 has connected").arg(pSocket->peerAddress().toString())
            .arg(pSocket->peerPort()));
}

void TheServer::onIoReadyRead()
{
    QTcpSocket* pSocket = static_cast<QTcpSocket*>(QObject::sender());
    if(pSocket == NULL) return;

    //read the incoming data
    QByteArray newData = pSocket->readAll();

    if(newData.isEmpty())
        return;

    //if the data fits the profile, create new clients
    if((newData.left(2)==QString("JH").toLocal8Bit() && newData.length() == VERSION1_LENGTH)
        || (newData.left(2)==QString("JH").toLocal8Bit() && newData.length() == VERSION2_LENGTH)
        || (newData.left(2)=="BG")
       ) {
        disconnect(pSocket, SIGNAL(readyRead()), this, SLOT(onIoReadyRead()));
        TcpClient *pClient = new TcpClient(pSocket);
        connect(this, SIGNAL(serverShutdown()), pClient, SLOT(onServerShutdown()));

        addClient(pClient);
    }
}

void TheServer::onClientDisconnected()
{
    TcpClient* pClient = static_cast<TcpClient*>(QObject::sender());

    Q_ASSERT(pClient!=NULL);

    //If this client was connected without ever giving an id, just remove it from list
    if(pClient->getClientState() == AClient::eUnknownState) {
        removeClient(pClient);
    }
}

//this is needed to get rid of dead clients with the same ID
void TheServer::onClientIDAssigned()
{
    AClient* pClient = static_cast<AClient*>(QObject::sender());
    //check to see if there is already a client with the same id
    for(int i=0; i<m_ClientList.size(); i++) {
        if(m_ClientList.at(i)->getClientId() == pClient->getClientId()) {
            //if there was one that's previously connected but offline, remove it
            AClient* pOldClient = m_ClientList.at(i);
            if(pOldClient->getClientState() == AClient::eOffline)
                removeClient(pOldClient);
        }
    }
}
