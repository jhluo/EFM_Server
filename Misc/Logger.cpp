#include "Logger.h"
#include <QDateTime>
#include <QDebug>
#include <QScrollBar>

Logger* Logger::pInstance = 0;

Logger* Logger::getInstance()
{
    if(pInstance == NULL) {
        pInstance = new Logger();
    }

    return pInstance;
}

Logger::Logger(QObject *parent):
    QObject(parent),
    m_DataLogEnabled(false),
    m_SystemLogEnabled(false),
    m_ShowTime(false),
    m_pLogDisplay(NULL),
    m_pDataDisplay(NULL),
    mMutex()
{

}


Logger::~Logger()
{
    delete pInstance;
}

void Logger::registerSystemDisplay(QTextEdit *pLogDisplay)
{
    m_pLogDisplay = pLogDisplay;
}

void Logger::registerDataDisplay(QTextEdit *pDataDisplay)
{
    m_pDataDisplay = pDataDisplay;
}

void Logger::write(const QString &line, const eLogType type)
{
    QMutexLocker locker(&mMutex);

    QString output(line);

    if (m_ShowTime) {
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:  ");
        output.prepend(time);
    }

    QTextEdit *pDisplay = NULL;
    QString filePath ="";
    bool logEnabled = false;

    if(type == eSystem) {
        pDisplay = m_pLogDisplay;
        filePath = m_LogFilePath;
        logEnabled = m_SystemLogEnabled;
    } else if(type == eData) {
        pDisplay = m_pDataDisplay;
        filePath = m_DataFilePath;
        logEnabled = m_DataLogEnabled;
    } else if(type == eDebug) {
        #ifdef QT_DEBUG
            qDebug() << line;
        #endif
    }

    writeToOutput(output, pDisplay);

    if(logEnabled)
        writeToOutput(output, filePath);
}

void Logger::writeToOutput(const QString &line, QTextEdit *pDisplay)
{
    if(pDisplay != NULL) {
        pDisplay->append(line);
        if(pDisplay->verticalScrollBar() != NULL)
            pDisplay->verticalScrollBar()->setSliderPosition(pDisplay->verticalScrollBar()->maximum());
    }
}

void Logger::writeToOutput(const QString &line, const QString &filePath)
{
    Q_UNUSED(line);
    Q_UNUSED(filePath);
}

void Logger::setSystemLogEnabled(const bool enable)
{
    m_SystemLogEnabled = enable;
}


void Logger::setDataLogEnabled(const bool enable)
{
    m_DataLogEnabled = enable;
}

void Logger::setShowTime(const bool enable)
{
    m_ShowTime = enable;
}

// End of file Logger.cpp
