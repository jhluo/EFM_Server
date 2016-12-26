#include "AClient.h"
#include "Misc/Logger.h"
#include "Misc/AppSettings.h"
#include "QScrollBar"
#include <QFile>
#include <QDir>
#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#define DATA_TIMEOUT 60000  //swith to no data state after 60 seconds
#define COMMAND_ACK_TIMEOUT 10000    //give 10 seconds for client to reply

#define FULL_DATA_SIZE 50    //size of data frame in bytes
#define PARTIAL_DATA_SIZE 37    //this is for the older version of data that only has 37 bytes

AClient::AClient(QObject *pParent)
    : QObject(pParent),
      m_ClientId("Unknown"),
      m_pInputDevice(NULL),
      m_ClientType(eUnknown),
      m_pDataViewer(NULL),
      m_ShowChart(false),
      m_lastCommandSent(0)
{
    m_DataBuffer.clear();

    m_ClientState = eOffline;

    //Time out the client if stop sending data
    m_pDataStarvedTimer = new QTimer(this);
    m_pDataStarvedTimer->setInterval(DATA_TIMEOUT);
    connect(m_pDataStarvedTimer, SIGNAL(timeout()), this, SLOT(onDataTimeout()));

    //Command acknowledgement timer
    m_pCommandAckTimer = new QTimer(this);
    m_pCommandAckTimer->setInterval(COMMAND_ACK_TIMEOUT);
    connect(m_pCommandAckTimer, SIGNAL(timeout()), this, SLOT(onCommandAckTimeout()));
}

AClient::~AClient()
{
    m_pDataStarvedTimer->stop();
    m_pCommandAckTimer->stop();
    m_DataBuffer.clear();
}

void AClient::setDataSource(QIODevice *pInputDevice, const eClientType type)
{
    //assign the socket to this client and connnect the slots
    m_pInputDevice = pInputDevice;
    m_pInputDevice->setParent(this);
    m_ClientType = type;

    connect(m_pInputDevice, SIGNAL(readyRead()), this, SLOT(onDataReceived()));
}

void AClient::registerDataViewer(QTextEdit *pTextEdit)
{
    m_pDataViewer = pTextEdit;
    if(m_pDataViewer!= NULL)
        connect(this, SIGNAL(outputMessage(QString)), m_pDataViewer, SLOT(append(QString)));
}

void AClient::connectClient()
{

}

void AClient::disconnectClient()
{

}

void AClient::setSerialConnect(const bool on)
{
    if(m_ClientType == eSerial) {
        if(on) {
            if(m_pInputDevice->open(QIODevice::ReadWrite)) {
                    m_ClientState = eNoData;
                    m_TimeOfConnect = QDateTime::currentDateTime();
                    m_pDataStarvedTimer->start();
                    emit clientDataChanged();
            } else {
                qDebug() << m_pInputDevice->errorString();
            }
        } else {
            m_pInputDevice->close();
            m_ClientState = eOffline;
            m_TimeOfDisconnect = QDateTime::currentDateTime();
            m_pDataStarvedTimer->stop();
            emit clientDataChanged();
        }
    }
}

QSerialPort* AClient::getClientSerialPort()
{
    if(m_ClientType==eSerial)
        return qobject_cast<QSerialPort*>(m_pInputDevice);

    return NULL;
}

void AClient::onDataReceived()
{
    //got data, refresh data timer
    m_pDataStarvedTimer->stop();
    m_ClientState = eOnline;

    //read the incoming data
    QByteArray newData = m_pInputDevice->readAll();

    if(newData.isEmpty())
        return;

    handleData(newData);

    //start timer again awaiting for next packet
    m_pDataStarvedTimer->start();
}

void AClient::sendCommand(const QString &data)
{
    int bytes = m_pInputDevice->write(data.toLocal8Bit());
    //qDebug() << QString("%1 bytes in buffer, %2 bytes are written").arg(data.toLocal8Bit().size()).arg(bytes);
    emit bytesSent(bytes);

    int indexColon = data.indexOf(":");
    int commandNum = data.mid(4, indexColon-4).toInt();
    m_lastCommandSent = commandNum;
    m_pCommandAckTimer->start();
}

void AClient::handleData(const QByteArray &newData)
{
    //handle command acknowledgement first, then return
    if(newData.left(3) == "ack") {
        bool ok = false;
        int commandNum = newData.mid(3, 2).toInt();
        QString id = newData.mid(6, 4);
        QString result = newData.right(5);

        if(commandNum == m_lastCommandSent
           && id == m_ClientId
           && result == "setok")
            ok = true;

        emit clientAcknowledge(ok);

        return;
    }


    if(newData.left(1) == "J" //it's a new packet
       || m_DataBuffer.size() >= FULL_DATA_SIZE // buffer is holding more than it should for some reason
       )
    {
        m_DataBuffer.clear(); //clear previous data
    }

    m_DataBuffer.append(newData);

    if(m_DataBuffer.size() == PARTIAL_DATA_SIZE) {    //old data format 37bit，add 13 bit make it 50
        for(int i=0; i<13; i++) {
            m_DataBuffer.append("0");
        }
    }

    if(m_DataBuffer.size() < FULL_DATA_SIZE) {    //not full packet, we wait till next iteration
        return;
    } else {
        m_DataBuffer.left(FULL_DATA_SIZE);
    }

    //qDebug() <<"Data to be decoded is:  " << m_Data.toHex()<<endl << endl;

    //Here we start decoding because we passed the checks above

    //first we validate the message header
    QString header;
    header.append(m_DataBuffer.left(5));
    if(header != "JHWTO") {
        m_DataBuffer.clear();
        return;
    }

    bool ok = false;
    if(m_ClientId == "Unknown") {//this is the first packet we get in this client, so tell server a new client has connected
        //this line is needed so that the slot knows what the ID is
        m_ClientId = QString::number(m_DataBuffer.mid(5, 2).toHex().toInt(&ok, 16));
        emit clientIDAssigned();

        //send an initial command to calibrate date
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QDate currentDate = currentDateTime.date();
        QTime currentTime = currentDateTime.time();
        QString command=QString("dxsj02:\"%1.%2.%3.%4.%5.%6.%7\"")
                .arg(currentDate.year()-2000)
                .arg(currentDate.month())
                .arg(currentDate.day())
                .arg(currentDate.dayOfWeek())
                .arg(currentTime.hour())
                .arg(currentTime.minute())
                .arg(currentTime.second());

        sendCommand(command);
        //qDebug() <<command;
    }

    m_ClientId = QString::number(m_DataBuffer.mid(5, 2).toHex().toInt(&ok, 16));

    int msgCount = m_DataBuffer.mid(7, 2).toHex().toInt(&ok, 16);
    Q_UNUSED(msgCount);

    QByteArray bcc = m_DataBuffer.mid(9, 1).toHex();
    Q_UNUSED(bcc);  //message checksum, no use yet


    if(m_DataBuffer.size() < 10)
       return;

    ClientData clientData;
    int second = m_DataBuffer.mid(10,1).toHex().toInt(&ok, 16);
    if(second > 59) second = 0;
    int month = m_DataBuffer.mid(11,1).toHex().toInt(&ok, 16);
    if(month > 12) month = 12;
    int day = m_DataBuffer.mid(12,1).toHex().toInt(&ok, 16);
    if(day > 31) day = 1;
    int hour = m_DataBuffer.mid(13,1).toHex().toInt(&ok, 16);
    if(hour>23) hour=0;
    int minute = m_DataBuffer.mid(14,1).toHex().toInt(&ok, 16);
    if(minute>59) minute=0;
    clientData.clientDate = QString("%1/%2/%3 %4:%5:%6")
                            .arg(2016)
                            .arg(month)
                            .arg(day)
                            .arg(hour)
                            .arg(minute)
                            .arg(second);

    //convert temperature to floating point number, first argument is higher byte,
    // second argument is lower byte
    clientData.temperature = convertToDecimal(m_DataBuffer.mid(15,1), m_DataBuffer.mid(16,1));

    //convert humidity to floating point number, same as temeprature
    clientData.humidity = convertToDecimal(m_DataBuffer.mid(17, 1), m_DataBuffer.mid(18,1));

    clientData.nIon = m_DataBuffer.mid(19, 2).toHex().toInt(&ok, 16);
    clientData.pIon = m_DataBuffer.mid(21, 2).toHex().toInt(&ok, 16);

    clientData.windDirection = m_DataBuffer.mid(23,2).toHex().toInt(&ok, 16);

    clientData.windSpeed = convertToDecimal(m_DataBuffer.mid(25,1), m_DataBuffer.mid(26,1));

    //convert rainfall to a double
    const char rain_h = m_DataBuffer.at(27);
    const char rain_l = m_DataBuffer.at(28);

    //shift higher-byte 4 bits to left then combine with first 4 bits of lower-byte
    //???this is different than what's in spec, need to verify
    int rain_int = (rain_h << 4) | ((rain_l & 0xF0) >> 4);

    //only look at last 4 bits of lower-byte
    int rain_dec = rain_l & 0x0F;

    //combine integer and fractional part
    double rain_frac = 0;
    if(rain_dec<10)
        rain_frac = rain_dec/10.0;
    else
        rain_frac = rain_dec/100.0;

    clientData.rainfall = rain_int + rain_frac;

    //pressure needs three bytes
    clientData.pressure = convertToDecimal(m_DataBuffer.mid(29, 2), m_DataBuffer.mid(31,1));

    clientData.ultraViolet = m_DataBuffer.mid(32, 2).toHex().toInt(&ok, 16);

    //TODO:  BCC check sum stuff on byte 34~36,  not implemented yet

    clientData.oxygen = m_DataBuffer.mid(37, 2).toHex().toInt(&ok, 16) / 10;

    clientData.pm1 = m_DataBuffer.mid(39, 2).toHex().toInt(&ok, 16) / 10;

    clientData.pm25 = m_DataBuffer.mid(41, 2).toHex().toInt(&ok, 16) / 10;

    clientData.pm10 = m_DataBuffer.mid(43, 2).toHex().toInt(&ok, 16) / 10;

    int error = 0;  //TBD
    Q_UNUSED(error);


    //emits signal for chart dialog
    emit receivedData(QDateTime::currentDateTime(), clientData.nIon);

    //emit signal to notify model
    emit clientDataChanged();

    //reply the client with ack command
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QDate currentDate = currentDateTime.date();
    QTime currentTime = currentDateTime.time();
    QString command=QString("dxsj32:%1%2%3%4%5")
            .arg(currentDate.year()-2000)
            .arg(currentDate.month())
            .arg(currentDate.day())
            .arg(currentTime.hour())
            .arg(currentTime.minute());

    sendCommand(command);

    //display the data if there's a viewer dialog opened
    if(m_pDataViewer != NULL) {
        QString DataStr = QString("ID: %1       "
                                  "Date: %2\n")
                                  .arg(m_ClientId)
                                  .arg(clientData.clientDate);

        if (clientData.temperature!=0){
                    DataStr += QString("Temperature【温度（℃）】:  %1\n" ).arg(clientData.temperature);
                }
        if (clientData.humidity!=0){
                    DataStr += QString("Humidity【湿度（%）】:  %1\n" ).arg(clientData.humidity);
                }
        if (clientData.nIon!=0){
                    DataStr += QString("Negative Ion【负离子（个/cm3）】:  %1\n" ).arg(clientData.nIon);
                }
        if (clientData.pIon!=0){
                    DataStr += QString("Positive Ion【正离子（个/cm3）】:  %1\n" ).arg(clientData.pIon);
                }
        if (clientData.windDirection!=0){
                    DataStr += QString("Wind Direction【风向（°）】:  %1\n" ).arg(clientData.windDirection);
                }
        if (clientData.windSpeed!=0){
                    DataStr += QString("Wind Speed【风速（m/s）】:  %1\n" ).arg(clientData.windSpeed);
                }
        if (clientData.rainfall!=0){
                    DataStr += QString("Rainfall【雨量（ml）】: %1\n" ).arg(clientData.rainfall);
                }
        if (clientData.pressure!=0){
                    DataStr += QString("Pressure【气压（Pa）】:  %1\n" ).arg(clientData.pressure);
                }
        if (clientData.ultraViolet!=0){
                    DataStr += QString("Ultraviolet【总辐射（W/m2）】:  %1\n" ).arg(clientData.ultraViolet);
                }
        if (clientData.oxygen!=0){
                    DataStr += QString("Oxygen Content【蒸发（mm）】: %1\n" ).arg(clientData.oxygen);
                }
        if (clientData.pm1!=0){
                    DataStr += QString("PM 1.0:  %1\n" ).arg(clientData.pm1);
                }
        if (clientData.pm25!=0){
                    DataStr += QString("PM 2.5:  %1\n" ).arg(clientData.pm25);
                }
        if (clientData.pm10!=0){
                    DataStr += QString("PM 10:  %1\n" ).arg(clientData.pm10);
                }

//        DataStr += QString("Temperature【温度】:  %1\n"
//                           "Humidity【湿度】:  %2\n"
//                           "Negative Ion【负离子】:  %3\n"
//                           "Positive Ion【正离子】:  %4\n"
//                           "Wind Direction【风向】:  %5\n"
//                           "Wind Speed【风速】:  %6\n"
//                           "Rain Fall【雨量】:  %7\n"
//                           "Pressure【气压】:  %8\n"
//                           "Ultra Violet【紫外线】:  %9\n"
//                           "Oxygen Concentration【含氧量】:  %10\n"
//                           "PM 1.0:  %11\n"
//                           "PM 2.5:  %12\n"
//                           "PM 10:  %13\n\n"
//                           )
//                           .arg(clientData.temperature)
//                           .arg(clientData.humidity)
//                           .arg(clientData.nIon)
//                           .arg(clientData.pIon)
//                           .arg(clientData.windDirection)
//                           .arg(clientData.windSpeed)
//                           .arg(clientData.rainfall)
//                           .arg(clientData.pressure)
//                           .arg(clientData.ultraViolet)
//                           .arg(clientData.oxygen)
//                           .arg(clientData.pm1)
//                           .arg(clientData.pm25)
//                           .arg(clientData.pm10);

        emit outputMessage(DataStr);
    }

    //write to log file, only do it if we have this enabled
    AppSettings settings;
    bool logEnabled = settings.readMiscSettings("dataLog", false).toBool();

    if(logEnabled) {
        if(!QDir("log").exists())
            QDir().mkdir("log");

        QString fileName = "log//" + m_ClientId + "_log.csv";
        writeDataLog(fileName, clientData);
    }

    bool rawLogEnabled = settings.readMiscSettings("rawLog", false).toBool();
    if(rawLogEnabled) {
        if(!QDir("log").exists())
            QDir().mkdir("log");

        QString fileName = "log//" + m_ClientId + "_raw.txt";
        writeRawLog(fileName, m_DataBuffer);
    }

    //try to connect to database, only if write to database enabled
    if(settings.readMiscSettings("writeDatabase", true).toBool()) {
        if(!writeDatabase(clientData)) {
            LOG_SYS(QString("Client %1 failed to write to database.").arg(m_ClientId));
        }
    }


    m_DataBuffer.clear(); //done decoding, clear the array
}


//disconnect the client when no data is being sent
void AClient::onDataTimeout()
{
    m_pDataStarvedTimer->stop();
    m_ClientState = eNoData;
    //emit signal to notify model
    emit clientDataChanged();
}

//throw message when client did not ack command
void AClient::onCommandAckTimeout()
{
    m_pCommandAckTimer->stop();
    emit error(QString(tr("Client %1 did not receive command.  Please retry.")).arg(m_ClientId));
}

void AClient::onSocketDisconnected()
{
    LOG_SYS(QString("Client %1 at %2 disconnected").arg(m_ClientId).arg(getClientAddress()));

    if(m_pDataStarvedTimer->isActive()) {
        m_pDataStarvedTimer->stop();
    }

    if(m_pCommandAckTimer->isActive()) {
        m_pCommandAckTimer->stop();
    }

    m_ClientState = eOffline;

    m_TimeOfDisconnect = QDateTime::currentDateTime();

    //emit signal to notify model
    emit clientDataChanged();

    //end the thread
    this->thread()->quit();
}

bool AClient::writeDatabase(const ClientData &data)
{
    bool result = false;

    AppSettings settings;
    QSqlDatabase db;

    int* threadId = (int *)(this->thread()->currentThreadId());
    m_ThreadId = QString::number(*id);

    if(!QSqlDatabase::contains(m_ThreadId)) {
        db = QSqlDatabase::addDatabase("QODBC", connectionName);
    } else {
        db = QSqlDatabase::database(connectionName);
    }

    QString dsn = QString("Driver={sql server};server=%1;database=%2;uid=%3;pwd=%4;")
            .arg(settings.readDatabaseSettings("host", "").toString())
            .arg(settings.readDatabaseSettings("DbName", "").toString())
            .arg(settings.readDatabaseSettings("user", "").toString())
            .arg(settings.readDatabaseSettings("password", "").toString());

    db.setDatabaseName(dsn);

    if(db.open()) {
        QString queryStr;
        queryStr = QString("INSERT INTO 分钟资料 (SationID, data_date, data_hour, data_Min, 浓度, 湿度, 温度, 正离子数, 风向, 风速, 雨量, 气压, 紫外线, 氧气含量, PM1, PM25, PM10, 错误标志)"
                           "VALUES (%1, '%2', %3, %4, %5, %6, %7, %8, %9, %10, %11, %12, %13, %14, %15, %16, %17, %18);"
                           )
                .arg(m_ClientId)
                .arg(data.clientDate)
                .arg(0)
                .arg(0)
                .arg(data.nIon)
                .arg(data.humidity)
                .arg(data.temperature)
                .arg(data.pIon)
                .arg(data.windDirection)
                .arg(data.windSpeed)
                .arg(data.rainfall)
                .arg(data.pressure)
                .arg(data.ultraViolet)
                .arg(data.oxygen)
                .arg(data.pm1)
                .arg(data.pm25)
                .arg(data.pm10)
                .arg(0);

        QSqlQuery query(db);
        result = query.exec(queryStr);
        if(result==false) {
            LOG_SYS("Insert failed\n");
            LOG_SYS("Date: "+data.clientDate+"\n");
            LOG_SYS(query.lastError().text());
        }

        queryStr = QString("UPDATE equipment "
                           "SET data_date='%1', 浓度=%2, 湿度=%3, 温度=%4, 正离子数=%5, 风向=%6, 风速=%7, 雨量=%8, 气压=%9, 紫外线=%10, 氧气含量=%11, PM1=%12, PM25=%13, PM10=%14"
                           "WHERE StationID=%15;"
                           )
                .arg(data.clientDate)
                .arg(data.nIon)
                .arg(data.humidity)
                .arg(data.temperature)
                .arg(data.pIon)
                .arg(data.windDirection)
                .arg(data.windSpeed)
                .arg(data.rainfall)
                .arg(data.pressure)
                .arg(data.ultraViolet)
                .arg(data.oxygen)
                .arg(data.pm1)
                .arg(data.pm25)
                .arg(data.pm10)
                .arg(m_ClientId);

        QSqlQuery updateQuery(db);
        result = updateQuery.exec(queryStr);
        if(result==false) {
            LOG_SYS("Update failed\n");
            LOG_SYS(updateQuery.lastError().text());
        }
        db.close();
    }


    //qDebug() <<query.lastError();
    //qDebug() <<query.lastQuery();

    return result;
}

void AClient::writeDataLog(const QString &fileName, const ClientData &data)
{
    QFile logFile(fileName);
    QTextStream stream(&logFile);

    //write the header if the file wasn't there before
    if(!logFile.exists()) {
        stream << "Client ID" << "," << "Time" << "," << "Temperature" << "," << "Humidity" << ","
               << "Negative Ion" << "," << "Positive Ion" << "," << "Wind Direction" << "," << "Wind Speed" << ","
               << "Rainfall" << "," << "Pressure" << "," << "Ultraviolet" << "," << "Oxygen" << ","
               << "PM1" << "," << "PM25" << "," << "PM10" << "\n";
    }

    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:MM:ss");
    QString dataStr = QString("%1, %2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12, %13, %14, %15\n")
                            .arg(m_ClientId)
                            .arg(currentTime)
                            .arg(data.temperature)
                            .arg(data.humidity)
                            .arg(data.nIon)
                            .arg(data.pIon)
                            .arg(data.windDirection)
                            .arg(data.windSpeed)
                            .arg(data.rainfall)
                            .arg(data.pressure)
                            .arg(data.ultraViolet)
                            .arg(data.oxygen)
                            .arg(data.pm1)
                            .arg(data.pm25)
                            .arg(data.pm10);

    if (logFile.open(QFile::WriteOnly|QFile::Append)) {
        stream << dataStr;
        logFile.close();
    }
}

void AClient::writeRawLog(const QString &fileName, const QByteArray &rawData)
{
    QFile rawFile(fileName);
    QTextStream stream(&rawFile);

    if (rawFile.open(QFile::WriteOnly|QFile::Append)) {
        QString rawStr;
        stream << rawStr.append(rawData.toHex() + "\n");
        rawFile.close();
    }
}

QString AClient::getClientState() const
{
    QString str = "";

    if(m_ClientState == eOnline)
        str = "Online";
    else if(m_ClientState == eNoData)
        str = "No Data";
    else if(m_ClientState == eOffline)
        str = "Offline";

    return str;
}

QString AClient::getClientAddress() const
{
    QString address = "";

    if(m_ClientType == eTcp) {
        QTcpSocket *pSocket = qobject_cast<QTcpSocket*>(m_pInputDevice);
        address = pSocket->peerAddress().toString();
    } else if (m_ClientType == eSerial) {
        QSerialPort *pPort = qobject_cast<QSerialPort*>(m_pInputDevice);
        address = pPort->portName();
    }

    return address;
}

QDateTime AClient::getClientConnectTime() const
{
    return m_TimeOfConnect;
}

QString AClient::getClientDisconnectTime() const
{
    if(!m_TimeOfDisconnect.isValid())
        return "";

    return m_TimeOfDisconnect.toString(QString("yyyy/MM/dd hh:mm:ss"));
}

QString AClient::getClientUpTime() const
{
    int seconds;

    if(m_ClientState == eOffline){
        seconds = m_TimeOfConnect.secsTo(m_TimeOfDisconnect);
    } else {
        seconds = m_TimeOfConnect.secsTo(QDateTime::currentDateTime());
    }

    if(seconds < 60)
        return QString("%1 sec").arg(seconds);

    if(seconds < 3600) {
        int minute = seconds / 60;
        int remain = seconds % 60;
        return QString("%1 min %2 sec").arg(minute).arg(remain);
    }

    int hour = seconds / 3600;
    int minSec = seconds % 3600;
    int minute = minSec / 60;

    return QString("%1 hr %2 min").arg(hour).arg(minute);
}

double AClient::convertToDecimal(const QByteArray &highByte, const QByteArray &lowByte)
{
    bool ok =false;
    int high = highByte.toHex().toInt(&ok, 16);
    int low = lowByte.toHex().toInt(&ok, 16);
    double decimal = 0;

    if(low < 100)
        decimal = low/100.0;
    else
        decimal = low/1000.0;

    double finalValue = 0;
    if(high >= 0)
        finalValue = high + decimal;
    else
        finalValue = high - decimal;

    return finalValue;
}

void AClient::setShowChart(const bool enabled)
{
    m_ShowChart = enabled;
}
