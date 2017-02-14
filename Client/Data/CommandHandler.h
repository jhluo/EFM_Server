#pragma once

#include <QObject>
#include <QIODevice>
#include <QTimer>

class CommandHandler : public QObject
{
    Q_OBJECT

public:
    explicit CommandHandler(QObject *pParent);
    ~CommandHandler();

    bool sendCommand(QIODevice *pOutputChannel, const QByteArray &command);
    void processCommand(const QString &command);

private:
    QTimer m_AckTimer;

private slots:
    void onAckTimeout();
};

