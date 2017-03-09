#include "CommandHandler.h"
#include "Client/AClient.h"

#define ACK_TIMEOUT 3000    //expect acknowledge to come back in 3 seconds
#define COMMAND_TIMER 60 * 1000 //send command every 1 minute (for version 3 only)

CommandHandler::CommandHandler(QIODevice* pInput, int version, QObject *pParent) :
    QObject(pParent),
    m_ClientVersion(version),
    m_pIoDevice(pInput),
    m_WaitingForReply(false)
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
    sendCommand(m_pIoDevice, "READDATA");
}

CommandHandler::~CommandHandler()
{
    m_pAckTimer->stop();
    m_pCommandTimer->stop();
}

bool CommandHandler::sendCommand(QIODevice *pOutputChannel, const QByteArray &command, const QString &expectedAck)
{
    if(pOutputChannel==NULL)
        return false;

    int bytes = pOutputChannel->write(command);
    if(expectedAck!="") {
        m_pAckTimer->start();
        m_WaitingForReply = true;
        m_ExpectedAck=expectedAck;
    }
    return (bytes == command.size());
}

void CommandHandler::processCommand(const QString &command)
{
    if(m_WaitingForReply) {
        if(command.contains(m_ExpectedAck)) {
            m_pAckTimer->stop();
            m_WaitingForReply=false;
            m_ExpectedAck="";
            emit commandAcknowledged(true);
        }
    }
}

void CommandHandler::onAckTimeout()
{
    emit commandAcknowledged(false);
}

void CommandHandler::onCommandTimeout()
{
    if(m_ClientVersion == static_cast<int>(AClient::eVersion3)) {
        sendCommand(m_pIoDevice, "READDATA");
    }
}
