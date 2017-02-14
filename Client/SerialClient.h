#pragma once

#include <QObject>
#include <QSerialPort>
#include "AClient.h"

class SerialClient : public AClient
{
    Q_OBJECT

public:
    SerialClient(QSerialPort *pPort, QObject *pParent = 0);
    virtual ~SerialClient();

    QIODevice *getInputDevice();

    QString getClientAddress() const;

    bool isSerial() const {return true;}

private:
    QSerialPort *m_pPort;

signals:
    void error(QString portError);

public slots:
    Q_INVOKABLE void connectClient();
    Q_INVOKABLE void disconnectClient();

private slots:

};

