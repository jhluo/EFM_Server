#include "CommandHandler.h"
#include "Client/AClient.h"

#define ACK_TIMEOUT 5000    //expect acknowledge to come back in 5 seconds
#define COMMAND_TIMER 60 * 1000 //send command every 1 minute (for version 3 only)

CommandHandler::CommandHandler(QIODevice* pInput, int version, QObject *pParent) :
    QObject(pParent),
    m_ClientVersion(version),
    m_pIoDevice(pInput)
{
    //time out command without acknowledgment
    m_pAckTimer = new QTimer(this);
    m_pAckTimer->setInterval(ACK_TIMEOUT);
    m_pAckTimer->setSingleShot(true);
    connect(m_pAckTimer, SIGNAL(timeout()), this, SLOT(onAckTimeout()));

    //command timer
    m_pCommandTimer = new QTimer(this);
    m_pCommandTimer->setInterval(COMMAND_TIMER);
    connect(m_pCommandTimer, SIGNAL(timeout()), this, SLOT(onCommandTimeout()));
    sendCommand(m_pIoDevice, "READDATA", false);
}

CommandHandler::~CommandHandler()
{
    m_pAckTimer->stop();
    m_pCommandTimer->stop();
}

bool CommandHandler::sendCommand(QIODevice *pOutputChannel, const QByteArray &command, const bool expectAck)
{
    if(pOutputChannel==NULL)
        return false;

    int bytes = pOutputChannel->write(command);
    if(expectAck)
        m_pAckTimer->start();
    return (bytes == command.size());
}

void CommandHandler::processCommand(const QString &command)
{
//    Q_UNUSED(command);
//    bool ok = false;
//    int commandNum = newData.mid(3, 2).toInt();
//    QString id = newData.mid(6, 4);
//    QString result = newData.right(5);

//    if(commandNum == m_lastCommandSent
//       && id == m_ClientId
//       && result == "setok")
//        ok = true;

//    m_pCommandAckTimer->stop();
//    emit clientAcknowledge(ok);
}

void CommandHandler::onAckTimeout()
{
    emit commandAcknowledged(false);
}

void CommandHandler::onCommandTimeout()
{
    if(m_ClientVersion == static_cast<int>(AClient::eVersion3)) {
        sendCommand(m_pIoDevice, "READDATA", false);
    }
}
