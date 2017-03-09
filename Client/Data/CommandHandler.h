#pragma once

#include <QObject>
#include <QIODevice>
#include <QTimer>

class CommandHandler : public QObject
{
    Q_OBJECT

public:
    explicit CommandHandler(QIODevice* pInput, int version = 0, QObject *pParent=0);
    ~CommandHandler();

    bool sendCommand(QIODevice *pOutputChannel, const QByteArray &command, const QString &expectedAck="");
    void processCommand(const QString &command);

private:
    //timer for acknowledging commands from client
    QTimer *m_pAckTimer;

    //timer for sending some commands periodically
    QTimer *m_pCommandTimer;

    int m_ClientVersion;

    QIODevice* m_pIoDevice;

    bool m_WaitingForReply;
    QString m_ExpectedAck;

signals:
    void commandAcknowledged(bool ok);

private slots:
    void onAckTimeout();
    void onCommandTimeout();
};

