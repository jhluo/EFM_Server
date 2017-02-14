#include "CommandHandler.h"

#define ACK_TIMEOUT 5000    //expect acknowledge to come back in 5 seconds

CommandHandler::CommandHandler(QObject *pParent) :
    QObject(pParent)
{
    //time out the data
    m_AckTimer.setInterval(ACK_TIMEOUT);
    m_AckTimer.setSingleShot(true);
    connect(&m_AckTimer, SIGNAL(timeout()), this, SLOT(onAckTimeout()));
}

CommandHandler::~CommandHandler()
{
    if(m_AckTimer.isActive())
        m_AckTimer.stop();
}

bool CommandHandler::sendCommand(QIODevice *pOutputChannel, const QByteArray &command)
{
    int bytes = pOutputChannel->write(command);
    m_AckTimer.start();
    return (bytes = command.size());
}

void CommandHandler::processCommand(const QString &command)
{
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

}
