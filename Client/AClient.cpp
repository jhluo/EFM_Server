#include "AClient.h"
#include "Misc/Logger.h"
#include "Misc/AppSettings.h"
#include "Misc/OffsetSettings.h"
#include "QScrollBar"
#include <QFile>
#include <QDir>
#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#define TIMER_INTERVAL 60000 * 2 //timer timeout interval

#define VERSION1_LENGTH 37  //length in bytes for fixed length messages
#define VERSION2_LENGTH 50

AClient::AClient(QObject *pParent)
    : QObject(pParent),
      m_pInputDevice(NULL),
      m_ClientId("Unknown"),
      m_pCommandHandler(NULL),
      m_pDataViewer(NULL),
      m_ShowChart(false)
{
    m_ClientState = eUnknownState;

    m_pClientData = new ClientData(this);

    m_ClientType = eUnknownType;

    m_pCommandHandler = new CommandHandler(this);

    //Timer to keep track of state of client
    m_pDataTimer = new QTimer(this);
    m_pDataTimer->setInterval(TIMER_INTERVAL);
    connect(m_pDataTimer, SIGNAL(timeout()), this, SLOT(onDataTimeout()));

    //send message to a logger
    connect(this, SIGNAL(error(QString)), Logger::getInstance(), SLOT(write(QString)));
}

AClient::~AClient()
{
    m_pDataTimer->stop();
}

void AClient::setDataSource(QIODevice *pInputDevice, const eClientType &type)
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

void AClient::setSerialConnect(const bool on)
{
    if(m_ClientType == eSerial) {
        if(on) {
            if(m_pInputDevice->open(QIODevice::ReadWrite)) {
                    m_ClientState = eNoData;
                    m_TimeOfConnect = QDateTime::currentDateTime();
                    m_pDataTimer->start();
                    emit clientDataChanged();
            } else {
                qDebug() << m_pInputDevice->errorString();
            }
        } else {
            m_pInputDevice->close();
            m_ClientState = eOffline;
            m_TimeOfDisconnect = QDateTime::currentDateTime();
            m_pDataTimer->stop();
            emit clientDataChanged();
        }
    }
}

void AClient::onDataReceived()
{
    //read the incoming data
    QByteArray newData = m_pInputDevice->readAll();

    if(newData.isEmpty())
        return;
    else if(newData.left(3) == "ack") {
        m_pCommandHandler->processCommand(newData);

        qDebug() << "Ack:  " << newData;
    } else {
        //got data, refresh data timer
        m_pDataTimer->stop();
        handleData(newData);
        //start timer again awaiting for next packet
        m_pDataTimer->start();
    }
}

void AClient::sendCommand(const QString &data)
{
    m_pCommandHandler->sendCommand(m_pInputDevice, data.toLocal8Bit());

    //qDebug() << QString("%1 bytes in buffer, %2 bytes are written").arg(data.toLocal8Bit().size()).arg(bytes);

//    int indexColon = data.indexOf(":");
//    int commandNum = data.mid(4, indexColon-4).toInt();
//    m_lastCommandSent = commandNum;
//    m_pCommandAckTimer->start();
}

void AClient::handleData(const QByteArray &newData)
{
    //verison 1 & 2
    if(newData.left(2)==QString("JH").toLocal8Bit()) {
        if(newData.length() == VERSION1_LENGTH) {
            m_ClientVersion = eVersion1;
            decodeVersion1Data(newData);
        } else if(newData.length() == VERSION2_LENGTH) {
            m_ClientVersion = eVersion2;
            decodeVersion2Data(newData);
        } else {
            return;
        }

    } else if(newData.left(2)=="BG") {
        m_ClientVersion = eVersion3;
        decodeVersion3Data(newData);
    } else {
        return;
    }

    //got legit data, refresh state
    m_ClientState = eOnline;

    //emits signal for chart dialog
    emit receivedData(QDateTime::currentDateTime(), m_pClientData->getData(ClientData::eNIon).toInt());

    //emit signal to notify model
    emit clientDataChanged();

    //reply the client with ack command
//    QDateTime currentDateTime = QDateTime::currentDateTime();
//    QDate currentDate = currentDateTime.date();
//    QTime currentTime = currentDateTime.time();
//    QString command=QString("dxsj32:%1%2%3%4%5")
//            .arg(currentDate.year()-2000)
//            .arg(currentDate.month())
//            .arg(currentDate.day())
//            .arg(currentTime.hour())
//            .arg(currentTime.minute());

//    sendCommand(command);

    writeDataViewer();

    //write to log file, only do it if we have this enabled
    AppSettings settings;
    bool logEnabled = settings.readMiscSettings("dataLog", false).toBool();

    if(logEnabled) {
        if(!QDir("log").exists())
            QDir().mkdir("log");

        QString fileName="";
        QString stationIdStr = QString::number(m_pClientData->getData(ClientData::eStationID).toInt());
        fileName += stationIdStr+"_";
        QString deviceIdStr = m_pClientData->getData(ClientData::eDeviceID).toString();
        fileName += deviceIdStr+"_";
        fileName += "Value_";
        QDateTime currTime;
        QString timeStr = currTime.currentDateTime().toString("yyyyMMddHH");
        fileName += timeStr;

        fileName = "log//" + fileName + ".txt";
        writeDataLog(fileName);
    }

    bool rawLogEnabled = settings.readMiscSettings("rawLog", false).toBool();
    if(rawLogEnabled) {
        if(!QDir("log").exists())
            QDir().mkdir("log");

        QString fileName = "log//" + m_ClientId + "_raw.txt";
        writeRawLog(fileName, newData);
    }

    //try to connect to database, only if write to database enabled
    if(settings.readMiscSettings("writeDatabase", true).toBool()) {
        if(!writeDatabase()) {
            emit error(QString("Client %1 failed to write to database.").arg(m_ClientId));
        }
    }
}

QVariant AClient::applyOffset(const QString &clientId, const ClientData::eDataId id, const QVariant &value)
{
    OffsetSettings settings;
    QVariant newValue(value);
    double base = settings.readBaseOffset(clientId);
    double multiplier = settings.readMultiplierOffset(clientId);

    switch(id) {
        case ClientData::eNIon:
            newValue.setValue(newValue.toDouble()*multiplier + base);
        break;

        default:
        break;
    }

    return newValue;
}

void AClient::decodeVersion1Data(const QByteArray &dataArray)
{
    //first we validate the message header
    QString header;
    header.append(dataArray.left(5));
    if(header != "JHWTO") {
        return;
    }

    bool ok = false;
    if(m_ClientId == "Unknown") {//this is the first packet we get in this client, so tell server a new client has connected
        //this line is needed so that the slot knows what the ID is
        m_ClientId = QString::number(dataArray.mid(5, 2).toHex().toInt(&ok, 16));
        emit clientIDAssigned();

        //send an initial command to calibrate date    
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QDate currentDate = currentDateTime.date();
        QString dateYear = currentDateTime.date().toString("yy");
        QString dateMonth = currentDateTime.date().toString("MM");
        QString dateDay = currentDateTime.date().toString("dd");
        QString timeHour = currentDateTime.time().toString("hh");
        QString timeMinute = currentDateTime.time().toString("mm");
        QString timeSecond = currentDateTime.time().toString("ss");
        QString command=QString("dxsj02:\"%1.%2.%3.%4.%5.%6.%7\"")
                .arg(dateYear)
                .arg(dateMonth)
                .arg(dateDay)
                .arg(currentDate.dayOfWeek())
                .arg(timeHour)
                .arg(timeMinute)
                .arg(timeSecond);

        sendCommand(command);
        //qDebug() <<command;
    }

    m_ClientId = QString::number(dataArray.mid(5, 2).toHex().toInt(&ok, 16));

    int msgCount = dataArray.mid(7, 2).toHex().toInt(&ok, 16);
    Q_UNUSED(msgCount);

    QByteArray bcc = dataArray.mid(9, 1).toHex();
    Q_UNUSED(bcc);  //message checksum, no use yet

    int second = dataArray.mid(10,1).toHex().toInt(&ok, 16);
    if(second > 59) second = 0;
    int month = dataArray.mid(11,1).toHex().toInt(&ok, 16);
    if(month > 12) month = 12;
    int day = dataArray.mid(12,1).toHex().toInt(&ok, 16);
    if(day > 31) day = 1;
    int hour = dataArray.mid(13,1).toHex().toInt(&ok, 16);
    if(hour>23) hour=0;
    int minute = dataArray.mid(14,1).toHex().toInt(&ok, 16);
    if(minute>59) minute=0;
    QString dataStr = QString("%1/%2/%3 %4:%5:%6")
                            .arg(2017)
                            .arg(month)
                            .arg(day)
                            .arg(hour)
                            .arg(minute)
                            .arg(second);

    m_pClientData->setData(ClientData::eClientDate, dataStr);

    //convert temperature to floating point number, first argument is higher byte,
    // second argument is lower byte
    m_pClientData->setData(ClientData::eTemperature, convertToDecimal(dataArray.mid(15,1), dataArray.mid(16,1)));

    //convert humidity to floating point number, same as temeprature
    m_pClientData->setData(ClientData::eHumidity, convertToDecimal(dataArray.mid(17, 1), dataArray.mid(18,1)));

    m_pClientData->setData(ClientData::eNIon, dataArray.mid(19, 2).toHex().toInt(&ok, 16));
    m_pClientData->setData(ClientData::ePIon, dataArray.mid(21, 2).toHex().toInt(&ok, 16));

    m_pClientData->setData(ClientData::eWindDirection, dataArray.mid(23,2).toHex().toInt(&ok, 16));

    m_pClientData->setData(ClientData::eWindSpeed, convertToDecimal(dataArray.mid(25,1), dataArray.mid(26,1)));

    //convert rainfall to a double
    const char rain_h = dataArray.at(27);
    const char rain_l = dataArray.at(28);

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

    m_pClientData->setData(ClientData::eRainfall, rain_int + rain_frac);

    //pressure needs three bytes
     m_pClientData->setData(ClientData::ePressure, convertToDecimal(dataArray.mid(29, 2), dataArray.mid(31,1)));

     m_pClientData->setData(ClientData::eUltraViolet, dataArray.mid(32, 2).toHex().toInt(&ok, 16));

    //TODO:  BCC check sum stuff on byte 34~36,  not implemented yet

    int error = 0;  //TBD
    Q_UNUSED(error);
}

void AClient::decodeVersion2Data(const QByteArray &dataArray)
{
    bool ok = false;
    decodeVersion1Data(dataArray);

    m_pClientData->setData(ClientData::eOxygen, dataArray.mid(37, 2).toHex().toInt(&ok, 16) / 10);

    m_pClientData->setData(ClientData::ePm1, dataArray.mid(39, 2).toHex().toInt(&ok, 16) / 10);

    m_pClientData->setData(ClientData::ePm25, dataArray.mid(41, 2).toHex().toInt(&ok, 16) / 10);

    m_pClientData->setData(ClientData::ePm10, dataArray.mid(43, 2).toHex().toInt(&ok, 16) / 10);

    m_pClientData->setData(ClientData::eError, dataArray.mid(45, 2).toHex().toInt(&ok, 16));
}

void AClient::decodeVersion3Data(const QByteArray &newData)
{
    m_pClientData->setData(ClientData::eStationID, newData.mid(3,5));
    m_pClientData->setData(ClientData::eLatitude, newData.mid(9,6).toInt());
    m_pClientData->setData(ClientData::eLongtitude, newData.mid(16,7).toInt());
    m_pClientData->setData(ClientData::eAltitude, newData.mid(24,5).toInt());
    m_pClientData->setData(ClientData::eServiceType, newData.mid(30,2).toInt());
    m_pClientData->setData(ClientData::eDeviceType, newData.mid(33,4));
    m_pClientData->setData(ClientData::eDeviceID, newData.mid(38,3));

    QString clientID = m_pClientData->getData(ClientData::eStationID).toString()+m_pClientData->getData(ClientData::eDeviceID).toString();

    if(m_ClientId == "Unknown") {//this is the first packet we get in this client, so tell server a new client has connected
        //this line is needed so that the slot knows what the ID is
        m_ClientId = clientID;
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

    m_ClientId = clientID;

    int year = newData.mid(42,4).toInt();
    int month = newData.mid(46,2).toInt();
    if(month > 12) month = 1;
    int day = newData.mid(48,2).toInt();
    if(day > 31) day = 1;
    int hour = newData.mid(50,2).toInt();
    if(hour>23) hour=0;
    int minute = newData.mid(52,2).toInt();
    if(minute>59) minute=0;
    int second = newData.mid(54,2).toInt();

    if(second>59) second=0;
    QString dateStr = QString("%1/%2/%3 %4:%5:%6")
                            .arg(year)
                            .arg(month)
                            .arg(day)
                            .arg(hour)
                            .arg(minute)
                            .arg(second);

    m_pClientData->setData(ClientData::eClientDate, dateStr);
    m_pClientData->setData(ClientData::eInterval, newData.mid(57,3).toInt());
    m_pClientData->setData(ClientData::eElementCount, newData.mid(61,3).toInt());
    m_pClientData->setData(ClientData::eStatusCount, newData.mid(65,2).toInt());


    //locate the index of the field
    int indexOfASA = newData.indexOf("ASA");
    int indexOfAAA5 = newData.indexOf("AAA5");
    int indexOfADA5 = newData.indexOf("ADA5");
    int indexOfASB = newData.indexOf("ASB");
    int indexOfASC = newData.indexOf("ASC");
    int indexOfASD = newData.indexOf("ASD");
    int indexOfASE = newData.indexOf("ASE");
    int indexOfASF = newData.indexOf("ASF");
    int indexOfASG = newData.indexOf("ASG");
    int indexOfz = newData.indexOf("z");
    int indexOfy_AAA = newData.indexOf("y_AAA");
    int indexOfy_ADA = newData.indexOf("y_ADA");
    int indexOfxA = newData.indexOf("xA");
    int indexOfxB = newData.indexOf("xB");
    int indexOfwA = newData.indexOf("wA");
    int indexOftQ = newData.indexOf("tQ");
//    int statusCode;
//    int qualityControl;



    //indexOfASA is where the field header "ASA" starts (at 68 in example)
    //indexOfASA +4 is after "ASA,".  From there we search the second , because we want ASA,xxxxxxx,
    //And then mid is the string between "ASA," and the second ",", which is the "xxxxxx".

    QString ASA_Str = "";
    if(indexOfASA != -1) {  //-1 is the case that "ASA" cannot not be found in the sentence
        ASA_Str = newData.mid(indexOfASA+4, newData.indexOf(",", indexOfASA+4)-(indexOfASA+4));
        int asaValue = applyOffset(m_ClientId, ClientData::eNIon, ASA_Str.toInt()).toInt();
        m_pClientData->setData(ClientData::eNIon, asaValue);
    }

    QString AAA5_Str = "";
    if(indexOfAAA5 != -1) {  //-1 is the case that "AAA5" cannot not be found in the sentence
        AAA5_Str = newData.mid(indexOfAAA5+5, newData.indexOf(",", indexOfAAA5+5)-(indexOfAAA5+5));
        m_pClientData->setData(ClientData::eTemperature, AAA5_Str.toInt() / 10.0);
    }

    QString ADA5_Str = "";
    if(indexOfADA5 != -1) {  //-1 is the case that "ASA" cannot not be found in the sentence
        ADA5_Str = newData.mid(indexOfADA5+5, newData.indexOf(",", indexOfADA5+5)-(indexOfADA5+5));
        m_pClientData->setData(ClientData::eHumidity, ADA5_Str);
    }

    QString ASB_Str = "";
    if(indexOfASB != -1) {  //-1 is the case that "ASB" cannot not be found in the sentence
        ASB_Str = newData.mid(indexOfASB+4, newData.indexOf(",", indexOfASB+4)-(indexOfASB+4));
        m_pClientData->setData(ClientData::ePolarVoltN, ASB_Str.toInt() / 10.0);
    }

    QString ASC_Str = "";
    if(indexOfASC != -1) {  //-1 is the case that "ASC" cannot not be found in the sentence
        ASC_Str = newData.mid(indexOfASC+4, newData.indexOf(",", indexOfASC+4)-(indexOfASC+4));
        m_pClientData->setData(ClientData::eRPML, ASC_Str.toInt());
    }

    QString ASD_Str = "";
    if(indexOfASD != -1) {  //-1 is the case that "ASD" cannot not be found in the sentence
        ASD_Str = newData.mid(indexOfASD+4, newData.indexOf(",", indexOfASD+4)-(indexOfASD+4));
        m_pClientData->setData(ClientData::eTubeTempL, ASD_Str.toInt() / 10.0);
    }

    QString ASE_Str = "";
    if(indexOfASE != -1) {  //-1 is the case that "ASE" cannot not be found in the sentence
        ASE_Str = newData.mid(indexOfASE+4, newData.indexOf(",", indexOfASE+4)-(indexOfASE+4));
        m_pClientData->setData(ClientData::eTubeHumidityL, ASE_Str.toInt()/1.0);
    }

    QString ASF_Str = "";
    if(indexOfASF != -1) {  //-1 is the case that "ASF" cannot not be found in the sentence
        ASF_Str = newData.mid(indexOfASF+4, newData.indexOf(",", indexOfASF+4)-(indexOfASF+4));
        m_pClientData->setData(ClientData::ePressure, ASF_Str.toInt()/10.0);
    }

    QString ASG_Str = "";
    if(indexOfASG != -1) {  //-1 is the case that "ASG" cannot not be found in the sentence
        ASG_Str = newData.mid(indexOfASG+4, newData.indexOf(",", indexOfASG+4)-(indexOfASG+4));
        m_pClientData->setData(ClientData::eInsulation, ADA5_Str.toInt());
    }

//    if(statusCode =-1)
//                    {
//                        switch(statusCode)
//                        {
//                        case 0:
//                            {
//                                status = QString(("正常"));
//                            }
//                            break;
//                        case 1:
//                            {
//                                status = QString(("异常"));
//                                }
//                            break;
//                        case 2:
//                            {
//                                status = QString(("故障"));
//                                }
//                            break;
//                        case 3:
//                            {
//                                status = QString(("偏高"));
//                                }
//                            break;
//                        case 4:
//                            {
//                                status = QString(("偏低"));
//                                }
//                            break;
//                        case 5:
//                            {
//                                status = QString(("停止"));
//                                }
//                            break;
//                        case 6:
//                            {
//                                status = QString(("交流"));
//                                }
//                            break;
//                        case 7:
//                            {
//                                status = QString(("直流"));
//                                }
//                            break;
//                        case 8:
//                            {
//                                status = QString(("未接外部电源"));
//                                }
//                            break;
//                        default:
//                            break;
//                        }
//                    }


//    if(qualityControl =-1)
//                    {
//                        switch(statusCode)
//                        {
//                        case 0:
//                            {
//                                qualityControl = QString(("正确"));
//                            }
//                            break;
//                        case 1:
//                            {
//                                qualityControl = QString(("可疑"));
//                                }
//                            break;
//                        case 2:
//                            {
//                                qualityControl = QString(("错误"));
//                                }
//                            break;
//                        case 3:
//                            {
//                                qualityControl = QString(("订正数据"));
//                                }
//                            break;
//                        case 4:
//                            {
//                                qualityControl = QString(("修改数据"));
//                                }
//                            break;
//                        case 5:
//                            {
//                                qualityControl = QString(("预留"));
//                                }
//                            break;
//                        case 6:
//                            {
//                                qualityControl = QString(("预留"));
//                                }
//                            break;
//                        case 7:
//                            {
//                                qualityControl = QString(("预留"));
//                                }
//                            break;
//                        case 8:
//                            {
//                                qualityControl = QString(("缺测"));
//                                }
//                            break;
//                        case 9:
//                            {
//                                qualityControl = QString(("未做质量控制"));
//                                }
//                            break;
//                        default:
//                            break;
//                        }
//                    }

    QString z_Str = "";
    if(indexOfz != -1) {  //-1 is the case that "z" cannot not be found in the sentence
        z_Str = newData.mid(indexOfz+2, newData.indexOf(",", indexOfz+2)-(indexOfz+2));

    }
    QString y_AAA_Str = "";
    if(indexOfy_AAA != -1) {  //-1 is the case that "y_AAA" cannot not be found in the sentence
        y_AAA_Str = newData.mid(indexOfy_AAA+6, newData.indexOf(",", indexOfy_AAA+6)-(indexOfy_AAA+6));

    }
    QString y_ADA_Str = "";
    if(indexOfy_ADA != -1) {  //-1 is the case that "y_ADA" cannot not be found in the sentence
        y_ADA_Str = newData.mid(indexOfy_ADA+6, newData.indexOf(",", indexOfy_ADA+6)-(indexOfy_ADA+6));

    }
    QString xA_Str = "";
    if(indexOfxA != -1) {  //-1 is the case that "xA" cannot not be found in the sentence
        xA_Str = newData.mid(indexOfxA+3, newData.indexOf(",", indexOfxA+3)-(indexOfxA+3));

    }
    QString xB_Str = "";
    if(indexOfxB != -1) {  //-1 is the case that "xB" cannot not be found in the sentence
        xB_Str = newData.mid(indexOfxB+3, newData.indexOf(",", indexOfxB+3)-(indexOfxB+3));

    }
    QString wA_Str = "";
    if(indexOfwA != -1) {  //-1 is the case that "wA" cannot not be found in the sentence
        wA_Str = newData.mid(indexOfwA+3, newData.indexOf(",", indexOfwA+3)-(indexOfwA+3));

    }

    QString tQ_Str = "";
    if(indexOftQ != -1) {  //-1 is the case that "tQ" cannot not be found in the sentence
        tQ_Str = newData.mid(indexOftQ+7, newData.indexOf(",", indexOftQ+3)-(indexOftQ+3));

    }
}

//disconnect the client when no data is being sent
void AClient::onDataTimeout()
{
    //first time it times out, set state to no data
    if(m_ClientState == eOnline) {
        m_ClientState = eNoData;
    } else if (m_ClientState == eNoData) {
        //second time it times out, it's a dead client and disconnect it
        disconnectClient();
        m_pDataTimer->stop();
    }

    //emit signal to notify model
    emit clientDataChanged();
}

bool AClient::writeDatabase()
{
    bool result = false;

    AppSettings settings;

    QSqlDatabase db;

    //QString connectionName = QString::number((int)(thread()->currentThreadId()));
    QString connectionName = m_ClientId;


    if(!db.contains(connectionName)) {
        db = QSqlDatabase::addDatabase("QODBC", connectionName);
        QString dsn = QString("Driver={sql server};server=%1;database=%2;uid=%3;pwd=%4;")

                .arg(settings.readDatabaseSettings("host", "").toString())
                .arg(settings.readDatabaseSettings("DbName", "").toString())
                .arg(settings.readDatabaseSettings("user", "").toString())
                .arg(settings.readDatabaseSettings("password", "").toString());

        db.setDatabaseName(dsn);
    } else {
        qDebug() << db.connectionNames();
        db = QSqlDatabase::database(connectionName);
    }

    if(db.open()) {
        QSqlQuery query(db);
        if(m_ClientVersion == eVersion1) {
            query.prepare("INSERT INTO 分钟资料 (SationID, data_date, data_hour, data_Min, 浓度, 湿度, 温度, 正离子数, 风向, 风速, 雨量, 气压, 紫外线, 氧气含量, PM1, PM25, PM10, 错误标志)"
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
            query.addBindValue(m_ClientId);
            query.addBindValue(m_pClientData->getData(ClientData::eClientDate));
            query.addBindValue(QVariant(QVariant::Int));
            query.addBindValue(QVariant(QVariant::Int));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eNIon).toInt()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eHumidity).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eTemperature).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::ePIon).toInt()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eWindDirection).toInt()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eWindSpeed).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eRainfall).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::ePressure).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eUltraViolet).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eOxygen).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::ePm1).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::ePm25).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::ePm10).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eError).toInt()));

//            query.addBindValue(m_ClientData.getData(ClientData::eNIon));
//            //query.addBindValue(QString("%1").arg(m_ClientData.getData(ClientData::eNIon).toInt(), 5, 10, QChar('0')));
//            query.addBindValue(m_ClientData.getData(ClientData::eHumidity));
//            query.addBindValue(m_ClientData.getData(ClientData::eTemperature));
//            query.addBindValue(m_ClientData.getData(ClientData::ePIon));
//            query.addBindValue(m_ClientData.getData(ClientData::eWindDirection));
//            query.addBindValue(m_ClientData.getData(ClientData::eWindSpeed));
//            query.addBindValue(m_ClientData.getData(ClientData::eRainfall));
//            query.addBindValue(m_ClientData.getData(ClientData::ePressure));
//            query.addBindValue(m_ClientData.getData(ClientData::eUltraViolet));
//            query.addBindValue(m_ClientData.getData(ClientData::eOxygen));
//            query.addBindValue(m_ClientData.getData(ClientData::ePm1));
//            query.addBindValue(m_ClientData.getData(ClientData::ePm25));
//            query.addBindValue(m_ClientData.getData(ClientData::ePm10));
            query.addBindValue(QVariant(QVariant::String));
        } else if (m_ClientVersion == eVersion2) {
            query.prepare("INSERT INTO 分钟资料 (SationID, data_date, data_hour, data_Min, 浓度, 湿度, 温度, 正离子数, 风向, 风速, 雨量, 气压, 紫外线, 氧气含量, PM1, PM25, PM10, 错误标志)"
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
            query.addBindValue(m_ClientId);
            query.addBindValue(m_pClientData->getData(ClientData::eClientDate));
            query.addBindValue(QVariant(QVariant::Int));
            query.addBindValue(QVariant(QVariant::Int));
            //query.addBindValue(m_ClientData.getData(ClientData::eNIon));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eNIon).toInt()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eHumidity).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eTemperature).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::ePIon).toInt()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eWindDirection).toInt()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eWindSpeed).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eRainfall).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::ePressure).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eUltraViolet).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eOxygen).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::ePm1).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::ePm25).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::ePm10).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eError).toInt()));

//            query.addBindValue(m_ClientData.getData(ClientData::eHumidity));
//            query.addBindValue(m_ClientData.getData(ClientData::eTemperature));
//            query.addBindValue(m_ClientData.getData(ClientData::ePIon));
//            query.addBindValue(m_ClientData.getData(ClientData::eWindDirection));
//            query.addBindValue(m_ClientData.getData(ClientData::eWindSpeed));
//            query.addBindValue(m_ClientData.getData(ClientData::eRainfall));
//            query.addBindValue(m_ClientData.getData(ClientData::ePressure));
//            query.addBindValue(m_ClientData.getData(ClientData::eUltraViolet));
//            query.addBindValue(m_ClientData.getData(ClientData::eOxygen));
//            query.addBindValue(m_ClientData.getData(ClientData::ePm1));
//            query.addBindValue(m_ClientData.getData(ClientData::ePm25));
//            query.addBindValue(m_ClientData.getData(ClientData::ePm10));
            query.addBindValue(QVariant(QVariant::String));
        } else if (m_ClientVersion == eVersion3) {
//            queryStr = QString("INSERT INTO 分钟资料 (区站号, SationID, data_date, data_hour, data_Min, 浓度, 湿度, 温度, 正离子数, 风向, 风速, 雨量, 气压, CO2, PM1, PM25, PM10, 测量室负温度, 测量室正温度, "
//                                        "甲醛, 极板负电压, 极板正电压, 风扇负转速, 风扇正转速, 关风机采集数, 开风机采集数, 关风机正离子, 开风机正离子, 经度, 纬度, 海拔高度, 服务类型, 设备标识, 帧标识, 设备标识码)"
//                               "VALUES (%1, '%2', '%3', %4, %5, %6, %7, %8, %9, %10, %11, %12, %13, %14, %15, %16, %17, %18, %19, %20, %21, %22, %23, %24, %25, %26, %27, %28, %29, %30, %31, %32, %33, %34);"
//                               )
//                    .arg(data.stationID)
//                    .arg(data.deviceString)
//                    .arg(0)
//                    .arg(0)
//                    .arg(data.nIon)
//                    .arg(data.humidity)
//                    .arg(data.temperature)
//                    .arg(data.pIon)
//                    .arg(data.windDirection)
//                    .arg(data.windSpeed)
//                    .arg(data.rainfall)
//                    .arg(data.pressure)
//                    .arg(data.CO2)
//                    .arg(data.pm1)
//                    .arg(data.pm25)
//                    .arg(data.pm10)
//                    .arg(data.TubeTempL)
//                    .arg(data.TubeTempR)
//                    .arg(data.VOC)
//                    .arg(data.PolarVoltN)
//                    .arg(data.PolarVoltP)
//                    .arg(data.RPML)
//                    .arg(data.RPMR)
//                    .arg(data.fanOffIonCountN)
//                    .arg(data.fanOnIonCountN)
//                    .arg(data.fanOffIonCountP)
//                    .arg(data.fanOnIonCountP)
//                    .arg(data.longtitude)
//                    .arg(data.latitude)
//                    .arg(data.altitude)
//                    .arg(data.serviceType)
//                    .arg(data.deviceType)
//                    .arg(data.interval)
//                    .arg(data.deviceID);

//            QString sHumidity;
//            QVariant Humidity = m_ClientData.getData(ClientData::eHumidity).toInt(), 3, 10, QChar('0');
//            if(!Humidity.isNull())
//            {
//                QString sHumidity = QString::number(Humidity.toInt());
//            }
            QString sWindDirection;
            QVariant WindDirection = m_pClientData->getData(ClientData::eWindDirection);
            if(!WindDirection.isNull())
            {
                QString sWindDirection = QString::number(WindDirection.toInt());
            }

            QString sWindSpeed;
            QVariant WindSpeed = m_pClientData->getData(ClientData::eWindSpeed);
            if(!WindSpeed.isNull())
            {
                QString sWindSpeed = QString::number(WindSpeed.toInt());
            }

            QString sPm25;
            QVariant Pm25 = m_pClientData->getData(ClientData::ePm25);
            if(!Pm25.isNull())
            {
                QString sPm25 = QString::number(Pm25.toInt());
            }

            QString sPm10;
            QVariant Pm10 = m_pClientData->getData(ClientData::ePm10);
            if(!Pm10.isNull())
            {
                QString sPm10 = QString::number(Pm10.toInt());
            }

            QString sRainfall;
            QVariant Rainfall = m_pClientData->getData(ClientData::eRainfall);
            if(!Rainfall.isNull())
            {
                QString sRainfall = QString::number(Rainfall.toInt());
            }

            QString sPressure;
            QVariant Pressure = m_pClientData->getData(ClientData::ePressure);
            if(!Pressure.isNull())
            {
                QString sPressure = QString::number(Pressure.toDouble());
            }

            QString sTubeTempL;
            QVariant TubeTempL = m_pClientData->getData(ClientData::eTubeTempL);
            if(!TubeTempL.isNull())
            {
                QString sTubeTempL = QString::number(TubeTempL.toDouble());
            }

            QString sTubeTempR;
            QVariant TubeTempR = m_pClientData->getData(ClientData::eTubeTempR);
            if(!TubeTempR.isNull())
            {
                QString STubeTempR = QString::number(TubeTempR.toDouble());
            }

            QString sPolarVoltN;
            QVariant PolarVoltN = m_pClientData->getData(ClientData::ePolarVoltN);
            if(!PolarVoltN.isNull())
            {
                QString sPolarVoltN = QString::number(PolarVoltN.toDouble());
            }

            QString sPolarVoltP;
            QVariant PolarVoltP = m_pClientData->getData(ClientData::ePolarVoltP);
            if(!PolarVoltP.isNull())
            {
                QString sPolarVoltP = QString::number(PolarVoltP.toDouble());
            }
            
            QString sRPML;
            QVariant RPML = m_pClientData->getData(ClientData::eRPML);
            if(!RPML.isNull())
            {
                QString sRPML = QString::number(RPML.toInt());
            }

            QString sRPMR;
            QVariant RPMR = m_pClientData->getData(ClientData::eRPMR);
            if(!RPMR.isNull())
            {
                QString sRPMR = QString::number(RPMR.toInt());
            }

            QString sFanOffIonCountN;
            QVariant FanOffIonCountN = m_pClientData->getData(ClientData::eFanOffIonCountN);
            if(!FanOffIonCountN.isNull())
            {
                QString sFanOffIonCountN = QString::number(FanOffIonCountN.toInt());
            }

            QString sFanOnIonCountN;
            QVariant FanOnIonCountN = m_pClientData->getData(ClientData::eFanOnIonCountN);
            if(!FanOnIonCountN.isNull())
            {
                QString sFanOnIonCountN = QString::number(FanOnIonCountN.toInt());
            }

            QString sFanOffIonCountP;
            QVariant FanOffIonCountP = m_pClientData->getData(ClientData::eFanOffIonCountP);
            if(!FanOffIonCountP.isNull())
            {
                QString sFanOffIonCountP = QString::number(FanOffIonCountP.toInt());
            }

            QString sFanOnIonCountP;
            QVariant FanOnIonCountP = m_pClientData->getData(ClientData::eFanOnIonCountP);
            if(!FanOnIonCountP.isNull())
            {
                QString sFanOnIonCountP = QString::number(FanOnIonCountP.toInt());
            }

            QString sLatitude;
            QVariant Latitude = m_pClientData->getData(ClientData::eLatitude);
            if(!Latitude.isNull())
            {
                QString sLatitude = QString::number(Latitude.toDouble());
            }

            QString sLongtitude;
            QVariant Longtitude = m_pClientData->getData(ClientData::eLongtitude);
            if(!Longtitude.isNull())
            {
                QString sLongtitude = QString::number(Longtitude.toDouble());
            }

            QString sAltitude;
            QVariant Altitude = m_pClientData->getData(ClientData::eAltitude);
            if(!Altitude.isNull())
            {
                QString sAltitude = QString::number(Altitude.toDouble());
            }

            QString sServiceType;
            QVariant ServiceType = m_pClientData->getData(ClientData::eServiceType);
            if(!ServiceType.isNull())
            {
                QString sServiceType = QString::number(FanOffIonCountP.toInt());
            }

            QString sDeviceType;
            QVariant DeviceType = m_pClientData->getData(ClientData::eDeviceType);
            if(!DeviceType.isNull())
            {
                QString sDeviceType = QString::number(DeviceType.toInt());
            }

            QString sInterval;
            QVariant Interval = m_pClientData->getData(ClientData::eInterval);
            if(!Interval.isNull())
            {
                QString sInterval = QString::number(Interval.toInt());
            }



            query.prepare("INSERT INTO 分钟资料 (区站号, SationID, data_date, data_hour, data_Min, 浓度, 湿度, 温度, 正离子数,"
                          "风向, 风速, 雨量, 气压, PM25, PM10, 测量室负温度, 测量室正温度, 极板负电压, 极板正电压, 风扇负转速,"
                          "风扇正转速, 关风机采集数, 开风机采集数, 关风机正离子, 开风机正离子, 经度, 纬度, 海拔高度, 服务类型, 设备标识,"
                          "帧标识, 设备标识码)"
                               "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);"
                         );
            query.addBindValue(m_pClientData->getData(ClientData::eStationID).toString());
            query.addBindValue(m_pClientData->getData(ClientData::eDeviceID).toString());
            query.addBindValue(m_pClientData->getData(ClientData::eClientDate));
            query.addBindValue(QVariant(QVariant::Int));
            query.addBindValue(QVariant(QVariant::Int));
            query.addBindValue(QVariant(QString("%1").arg(m_pClientData->getData(ClientData::eNIon).toInt(), 6, 10, QChar('0'))));
            query.addBindValue(QVariant(QString("%1").arg(m_pClientData->getData(ClientData::eHumidity).toInt(), 3, 10, QChar('0'))));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eHumidity).toDouble()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eTemperature).toDouble()));
            query.addBindValue(QString("%1").arg(m_pClientData->getData(ClientData::ePIon).toInt(), 6, 10, QChar('0')));
            query.addBindValue(QVariant(QString(sWindDirection)));
            //query.addBindValue(QVariant(QString::number(m_ClientData.getData(ClientData::eWindDirection).toInt())));
            query.addBindValue(QVariant(QString(sWindSpeed)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eWindSpeed).toDouble()));
            query.addBindValue(QVariant(QString(sRainfall)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eRainfall).toInt()));
            query.addBindValue(QVariant(QString(sPressure)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::ePressure).toDouble()));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eCO2).toInt()));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::ePm1).toInt()));
            query.addBindValue(QVariant(QString(sPm25)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::ePm25).toInt()));
            query.addBindValue(QVariant(QString(sPm10)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::ePm10).toInt()));


            query.addBindValue(QVariant(QString(sTubeTempL)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eTubeTempL).toDouble()));
            query.addBindValue(QVariant(QString(sTubeTempR)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eTubeTempR).toDouble()));
            query.addBindValue(QVariant(QString(sPolarVoltN)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::ePolarVoltN).toDouble()));
            query.addBindValue(QVariant(QString(sPolarVoltP)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::ePolarVoltP).toDouble()));
           // query.addBindValue(QString::number(m_ClientData.getData(ClientData::eRPML).toInt()));
            query.addBindValue(QVariant(QString(sRPML)));
            query.addBindValue(QVariant(QString(sRPMR)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eRPMR).toInt()));
            query.addBindValue(QVariant(QString(sFanOffIonCountN)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eFanOffIonCountN).toInt()));
            query.addBindValue(QVariant(QString(sFanOnIonCountN)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eFanOnIonCountN).toInt()));
            query.addBindValue(QVariant(QString(sFanOffIonCountP)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eFanOffIonCountP).toInt()));
            query.addBindValue(QVariant(QString(sFanOnIonCountP)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eFanOnIonCountP).toInt()));
            query.addBindValue(QVariant(QString(sLatitude)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eLatitude).toDouble()));
            query.addBindValue(QVariant(QString(sLongtitude)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eLongtitude).toDouble()));
            query.addBindValue(QVariant(QString(sAltitude)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eAltitude).toDouble()));
            query.addBindValue(QVariant(QString(sServiceType)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eServiceType).toInt()));
            query.addBindValue(QVariant(QString(sDeviceType)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eDeviceType).toInt()));
            query.addBindValue(QVariant(QString(sInterval)));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eInterval).toInt()));
            query.addBindValue(QString::number(m_pClientData->getData(ClientData::eDeviceID).toInt()));
            //query.addBindValue(QString::number(m_ClientData.getData(ClientData::eVOC).toInt()));


            //query.addBindValue(m_ClientData.getData(ClientData::eTemperature));

            //query.addBindValue(m_ClientData.getData(ClientData::ePIon));

        }

        qDebug() << "Sataion" << m_pClientData->getData(ClientData::eStationID);
        qDebug() << "DeviceID" << m_pClientData->getData(ClientData::eDeviceID);
        qDebug() << "Date" << m_pClientData->getData(ClientData::eClientDate);
//        qDebug() << "Negative Ion" << QVariant(QString("%1").arg(m_ClientData.getData(ClientData::eNIon).toInt(), 6, 10, QChar('0')));
//        qDebug() << "Humidity" << QString("%1").arg(m_ClientData.getData(ClientData::eHumidity).toInt(), 3, 10, QChar('0'));
//        //qDebug() << "Temp" << m_ClientData.getData(ClientData::eTemperature);
//        qDebug() << "Temp" << QVariant(QString::number(m_ClientData.getData(ClientData::eTemperature).toDouble()));
//        //qDebug() << "Sataion" << m_ClientData.getData(ClientData::eStationID);
        qDebug() << "Device ID" << m_pClientData->getData(ClientData::eDeviceID).toInt();
        qDebug() << "Wind Direction" << QVariant(QString::number(m_pClientData->getData(ClientData::eWindDirection).toInt()));
        qDebug() << "Wind Speed" << m_pClientData->getData(ClientData::eWindSpeed).toDouble();
        

        result = query.exec();
        if(result==false) {
            qDebug() << "Insert failed\n";
            qDebug() << query.lastError().text();
        }

        db.close();
    } else {
        qDebug() << "Database failed to open in AClient::writeToDatabase\n";
    }


    //qDebug() <<query.lastError();
    //qDebug() <<query.lastQuery();

    return result;
}


void AClient::writeDataViewer()
{
    //display the data if there's a viewer dialog opened
    if(m_pDataViewer != NULL) {
        QString DataStr = QString("ID: %1       "
                                  "Date: %2\n")
                                  .arg(m_ClientId)
                                  .arg(m_pClientData->getData(ClientData::eClientDate).toString());


        if (!m_pClientData->getData(ClientData::eTemperature).isNull())
            DataStr += QString("Temperature【温度（℃）】:  %1\n" ).arg(m_pClientData->getData(ClientData::eTemperature).toDouble());

        if (!m_pClientData->getData(ClientData::eHumidity).isNull())
            DataStr += QString("Humidity【湿度（%）】:  %1\n" ).arg(m_pClientData->getData(ClientData::eHumidity).toDouble());

        if (!m_pClientData->getData(ClientData::eNIon).isNull())
            DataStr += QString("Negative Ion【负离子（个/cm3）】:  %1\n" ).arg(m_pClientData->getData(ClientData::eNIon).toInt());

        if (!m_pClientData->getData(ClientData::ePIon).isNull())
            DataStr += QString("Positive Ion【正离子（个/cm3）】:  %1\n" ).arg(m_pClientData->getData(ClientData::ePIon).toInt());

        if (!m_pClientData->getData(ClientData::eWindDirection).isNull())
            DataStr += QString("Wind Direction【风向（°）】:  %1\n" ).arg(m_pClientData->getData(ClientData::eWindDirection).toInt());

        if (!m_pClientData->getData(ClientData::eWindSpeed).isNull())
            DataStr += QString("Wind Direction【风速（m/s）】:  %1\n" ).arg(m_pClientData->getData(ClientData::eWindSpeed).toDouble());

        if (!m_pClientData->getData(ClientData::eRainfall).isNull())
            DataStr += QString("Rainfall【雨量（ml）】:  %1\n" ).arg(m_pClientData->getData(ClientData::eRainfall).toInt());

        if (!m_pClientData->getData(ClientData::ePressure).isNull())
            DataStr += QString("Pressure【气压（Pa）】:  %1\n" ).arg(m_pClientData->getData(ClientData::ePressure).toDouble());

        if (!m_pClientData->getData(ClientData::eCO2).isNull())
            DataStr += QString("CO2【二氧化碳（PPM）】:  %1\n" ).arg(m_pClientData->getData(ClientData::eCO2).toInt());

        if (!m_pClientData->getData(ClientData::eRainfall).isNull())
            DataStr += QString("PM 2.5 (ug/m3):  %1\n" ).arg(m_pClientData->getData(ClientData::eRainfall).toInt());

        if (!m_pClientData->getData(ClientData::eRainfall).isNull())
            DataStr += QString("PM 10 (ug/m3):  %1\n" ).arg(m_pClientData->getData(ClientData::eRainfall).toInt());

        if (!m_pClientData->getData(ClientData::eTubeTempL).isNull())
            DataStr += QString("Polar Voltage Negative【极板负电压（V）】:  %1\n" ).arg(m_pClientData->getData(ClientData::eTubeTempL).toDouble());

        if (!m_pClientData->getData(ClientData::eTubeTempR).isNull())
            DataStr += QString("Polar Voltage Negative【极板正电压（V）】:  %1\n" ).arg(m_pClientData->getData(ClientData::eTubeTempR).toDouble());

        if (!m_pClientData->getData(ClientData::ePolarVoltN).isNull())
            DataStr += QString("Tube Temperature Left【左管温度（℃）】:  %1\n" ).arg(m_pClientData->getData(ClientData::ePolarVoltN).toDouble());

        if (!m_pClientData->getData(ClientData::ePolarVoltP).isNull())
            DataStr += QString("Tube Temperature Left【右管温度（℃）】:  %1\n" ).arg(m_pClientData->getData(ClientData::ePolarVoltP).toDouble());

        if (!m_pClientData->getData(ClientData::eRPML).isNull())
            DataStr += QString("Fan Speed Left【左风扇转速（转/s）】:  %1\n" ).arg(m_pClientData->getData(ClientData::eRPML).toInt());

        if (!m_pClientData->getData(ClientData::eRPMR).isNull())
            DataStr += QString("Fan Speed Right【右风扇转速（转/s）】:  %1\n" ).arg(m_pClientData->getData(ClientData::eRPMR).toInt());

        if (!m_pClientData->getData(ClientData::eFanOffIonCountN).isNull())
            DataStr += QString("Fan Off Negative Ion Count 【关风机负离子采集数】:  %1\n" ).arg(m_pClientData->getData(ClientData::eFanOffIonCountN).toInt());

        if (!m_pClientData->getData(ClientData::eFanOnIonCountN).isNull())
            DataStr += QString("Fan On Negative Ion Count 【开风机负离子采集数】:  %1\n" ).arg(m_pClientData->getData(ClientData::eFanOnIonCountN).toInt());

        if (!m_pClientData->getData(ClientData::eFanOffIonCountP).isNull())
            DataStr += QString("Fan Off Positive Ion Count 【关风机正离子采集数】:  %1\n" ).arg(m_pClientData->getData(ClientData::eFanOffIonCountP).toInt());

        if (!m_pClientData->getData(ClientData::eFanOnIonCountP).isNull())
            DataStr += QString("Fan On Positive Ion Count 【开风机正离子采集数】:  %1\n" ).arg(m_pClientData->getData(ClientData::eFanOnIonCountP).toInt());

        if (!m_pClientData->getData(ClientData::eLatitude).isNull())
            DataStr += QString("Latitude 【纬度（°）】:  %1\n" ).arg(m_pClientData->getData(ClientData::eLatitude).toDouble());

        if (!m_pClientData->getData(ClientData::eLongtitude).isNull())
            DataStr += QString("Longtitude【经度（°）】:  %1\n" ).arg(m_pClientData->getData(ClientData::eLongtitude).toDouble());

        if (!m_pClientData->getData(ClientData::eAltitude).isNull())
            DataStr += QString("Altitude【海拔（米）】:  %1\n" ).arg(m_pClientData->getData(ClientData::eAltitude).toDouble());

        if (!m_pClientData->getData(ClientData::eServiceType).isNull())
            DataStr += QString("Service Type 【服务类型】:  %1\n" ).arg(m_pClientData->getData(ClientData::eServiceType).toInt());

        if (!m_pClientData->getData(ClientData::eDeviceType).isNull())
            DataStr += QString("Device Type 【设备标识】:  %1\n" ).arg(m_pClientData->getData(ClientData::eDeviceType).toInt());

        if (!m_pClientData->getData(ClientData::eInterval).isNull())
            DataStr += QString("Interval 【帧标识】:  %1\n" ).arg(m_pClientData->getData(ClientData::eInterval).toInt());

        if (!m_pClientData->getData(ClientData::eDeviceID).isNull())
            DataStr += QString("Device ID 【设备标识码】:  %1\n" ).arg(m_pClientData->getData(ClientData::eDeviceID).toString());



//        if (clientData.pIon!=0){
//                    DataStr += QString("Positive Ion【正离子（个/cm3）】:  %1\n" ).arg(clientData.pIon);
//                }
//        if (clientData.windDirection!=0){
//                    DataStr += QString("Wind Direction【风向（°）】:  %1\n" ).arg(clientData.windDirection);
//                }
//        if (clientData.windSpeed!=0){
//                    DataStr += QString("Wind Speed【风速（m/s）】:  %1\n" ).arg(clientData.windSpeed);
//                }
//        if (clientData.rainfall!=0){
//                    DataStr += QString("Rainfall【雨量（ml）】: %1\n" ).arg(clientData.rainfall);
//                }
//        if (clientData.pressure!=0){
//                    DataStr += QString("Pressure【气压（Pa）】:  %1\n" ).arg(clientData.pressure);
//                }
//        if (clientData.ultraViolet!=0){
//                    DataStr += QString("Ultraviolet【总辐射（W/m2）】:  %1\n" ).arg(clientData.ultraViolet);
//                }
//        if (clientData.oxygen!=0){
//                    DataStr += QString("Oxygen Content【蒸发（mm）】: %1\n" ).arg(clientData.oxygen);
//                }
//        if (clientData.pm1!=0){
//                    DataStr += QString("PM 1.0 (ug/m3):  %1\n" ).arg(clientData.pm1);
//                }
//        if (clientData.pm25!=0){
//                    DataStr += QString("PM 2.5 (ug/m3):  %1\n" ).arg(clientData.pm25);
//                }
//        if (clientData.pm10!=0){
//                    DataStr += QString("PM 10 (ug/m3):  %1\n" ).arg(clientData.pm10);
//                }
//        if (clientData.CO2!=0){
//                    DataStr += QString("CO2【二氧化碳】:  %1\n" ).arg(clientData.CO2);
//                }
//        if (clientData.VOC!=0){
//                    DataStr += QString("VOC【甲醛】:  %1\n" ).arg(clientData.VOC);
//                }
//        if (clientData.PolarVoltP!=0){
//                    DataStr += QString("Polar Voltage Positive【极板正电压（V）】:  %1\n" ).arg(clientData.PolarVoltP);
//                }
//        if (clientData.PolarVoltN!=0){
//                    DataStr += QString("Polar Voltage Negative【极板正电压（V）】:  %1\n" ).arg(clientData.PolarVoltN);
//                }
//        if (clientData.TubeTempL!=0){
//                    DataStr += QString("Tube Temperature Left【左管温度（℃）】:  %1\n" ).arg(clientData.TubeTempL);
//                }
//        if (clientData.TubeTempR!=0){
//                    DataStr += QString("Tube Temperature Right【右管温度（℃）】:  %1\n" ).arg(clientData.TubeTempR);
//                }
//        if (clientData.RPML!=0){
//                    DataStr += QString("Fan Speed Left【左风扇转速（转/s）】:  %1\n" ).arg(clientData.RPML);
//                }
//        if (clientData.RPMR!=0){
//                    DataStr += QString("Fan Speed Right【右风扇转速（转/s）】:  %1\n" ).arg(clientData.RPMR);
//                }
//        if (clientData.fanOnIonCountN!=0){
//                    DataStr += QString("Fan On Negative Ion Count 【开风机负离子采集数】:  %1\n" ).arg(clientData.fanOnIonCountN);
//                }
//        if (clientData.fanOffIonCountN!=0){
//                    DataStr += QString("Fan Off Negative Ion Count 【关风机负离子采集数】:  %1\n" ).arg(clientData.fanOffIonCountN);
//                }
//        if (clientData.fanOnIonCountP!=0){
//                    DataStr += QString("Fan On Positive Ion Count 【开风机正离子采集数】:  %1\n" ).arg(clientData.fanOnIonCountP);
//                }
//        if (clientData.fanOffIonCountP!=0){
//                    DataStr += QString("Fan Off Positive Ion Count 【关风机正离子采集数】:  %1\n" ).arg(clientData.fanOffIonCountP);
//                }
//        if (clientData.interval!=0){
//                    DataStr += QString("Interval【帧标识】:  %1\n" ).arg(clientData.interval);
//                }
//        if (clientData.elementCount!=0){
//                    DataStr += QString("Element Count【观测要素】:  %1\n" ).arg(clientData.elementCount);
//                }
//        if (clientData.statusCount!=0){
//                    DataStr += QString("Status Count【状态要素】:  %1\n" ).arg(clientData.statusCount);
//                }

        emit outputMessage(DataStr);
    }
}

void AClient::writeDataLog(const QString &fileName)
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
                            .arg(m_pClientData->getData(ClientData::eTemperature).toDouble())
                            .arg(m_pClientData->getData(ClientData::eHumidity).toDouble())
                            .arg(m_pClientData->getData(ClientData::eNIon).toInt())
                            .arg(m_pClientData->getData(ClientData::ePIon).toInt())
                            .arg(m_pClientData->getData(ClientData::eWindDirection).toDouble())
                            .arg(m_pClientData->getData(ClientData::eWindSpeed).toDouble())
                            .arg(m_pClientData->getData(ClientData::eRainfall).toDouble())
                            .arg(m_pClientData->getData(ClientData::ePressure).toDouble())
                            .arg(m_pClientData->getData(ClientData::eUltraViolet).toDouble())
                            .arg(m_pClientData->getData(ClientData::eOxygen).toDouble())
                            .arg(m_pClientData->getData(ClientData::ePm1).toDouble())
                            .arg(m_pClientData->getData(ClientData::ePm25).toDouble())
                            .arg(m_pClientData->getData(ClientData::ePm10).toDouble());

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
        if(m_ClientVersion == eVersion1 || m_ClientVersion == eVersion2)
            stream << rawStr.append(rawData.toHex() + "\n");
        else
            stream << rawStr.append(rawData + "\n");
        rawFile.close();
    }
}

//AClient::eClientState AClient::getClientState() const
//{
//    QString str = "";

//    if(m_ClientState == eOnline)
//        str = "Online";
//    else if(m_ClientState == eNoData)
//        str = "No Data";
//    else if(m_ClientState == eOffline)
//        str = "Offline";

//    return str;
//}

//QString AClient::getClientDisconnectTime() const
//{
//    if(!m_TimeOfDisconnect.isValid())
//        return "";

//    return m_TimeOfDisconnect.toString(QString("yyyy/MM/dd hh:mm:ss"));
//}

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
