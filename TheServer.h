#ifndef THESERVER_H
#define THESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QList>

class AClient;

class TheServer : public QTcpServer
{
    Q_OBJECT

public:
    TheServer(QObject *pParent = 0);
    ~TheServer();

    void startServer();
    void shutdownServer();

    int getTotalClient() const;
    QList<AClient *> *getClientList();
    AClient *getClient(const int index);

signals:

private:
    //all client connections are added to this list
    QList<AClient*> m_ClientList;

private slots:
    void onNewConnection();
    void onNewClientConnected();
};

#endif // THESERVER_H
