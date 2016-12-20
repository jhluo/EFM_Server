#ifndef THESERVER_H
#define THESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QSerialPort>
#include <QList>
#include "AClientList.h"

class TheServer : public QTcpServer
{
    Q_OBJECT

public:
    TheServer(QObject *pParent = 0);
    ~TheServer();

    void startServer();
    void shutdownServer();

    AClientList *getClientList() {return m_pClientList;}

signals:

private:
    //all client connections are added to this list
    AClientList *m_pClientList;

public slots:
    //add client from serial port connection
    void addSerialClient(QSerialPort *pPort);

private slots:
    void onNewConnection();
    void onNewClientConnected();
};

#endif // THESERVER_H
