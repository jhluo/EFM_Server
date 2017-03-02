#ifndef THESERVER_H
#define THESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QSerialPort>

class AClient;

class TheServer : public QTcpServer
{
    Q_OBJECT

public:
    TheServer(QObject *pParent = 0);
    ~TheServer();

    void startServer(const QString &host="127.0.0.1", const int port=5101);
    void shutdownServer();

    //add client from serial port connection
    void addSerialClient(QSerialPort *pPort);

    void addClient(AClient* pClient);
    void removeClient(AClient* pClient);
    //void sortClients(const int column, Qt::SortOrder order);

    int getClientCount() const;
    QStringList getClientList() const;
    AClient* getClient(const QString &key) const;

signals:
    void clientAdded(const QString &key);
    void clientRemoved(const QString &key);
    void clientDataChanged(const QString &key);
    void serverShutdown();

private:
    //all client connections are added to this list
    //AClientList *m_pClientList;
    QHash<QString, AClient*> m_ClientHash;

public slots:


private slots:
    //add Tcpip Client
    void onNewTcpClientConnected();
    void onClientDisconnected();
    void onClientIDAssigned();
};

#endif // THESERVER_H
