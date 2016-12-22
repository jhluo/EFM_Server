#pragma once

#include <QObject>
#include <QTcpSocket>
#include "AClient.h"

class TcpClient : public AClient
{
    Q_OBJECT

public:
    TcpClient(QTcpSocket *pSocket, QObject *pParent = 0);
    virtual ~TcpClient();

private:
    QTcpSocket *m_pSocket;

signals:
    void error(QTcpSocket::SocketError socketerror);

public slots:
    Q_INVOKABLE void disconnectClient();

private slots:
    void onSocketDisconnected();
};
