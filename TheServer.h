#ifndef THESERVER_H
#define THESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QSerialPort>

class AClientList;

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

    AClientList *getClientList() {return m_pClientList;}

signals:

private:
    //all client connections are added to this list
    AClientList *m_pClientList;

public slots:


private slots:
    //add Tcpip Client
    void onNewTcpClientConnected();


    void onTcpClientDisconnected();
    void onClientIDAssigned();
};

#endif // THESERVER_H
