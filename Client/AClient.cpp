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

#define VERSION1_LENGTH 37  //length in bytes for fixed length messages
#define VERSION2_LENGTH 50

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

    m_DataBuffer.clear();
    m_DataBuffer.append(newData);

    ClientData clientData;
    //verison 1 & 2
    if(m_DataBuffer.left(2)=="JH") {
        if(newData.length() == VERSION1_LENGTH) {
            m_ClientVersion = eVersion1;
            decodeVersion1Data(m_DataBuffer, clientData);
        } else if(newData.length() == VERSION2_LENGTH) {
            m_ClientVersion = eVersion2;
            decodeVersion2Data(m_DataBuffer, clientData);
        }
    } else if(newData.left(2)=="BG") {
        m_ClientVersion = eVersion3;
        decodeVersion3Data(m_DataBuffer, clientData);
    } else {
        return;
    }

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
                    DataStr += QString("PM 1.0 (ug/m3):  %1\n" ).arg(clientData.pm1);
                }
        if (clientData.pm25!=0){
                    DataStr += QString("PM 2.5 (ug/m3):  %1\n" ).arg(clientData.pm25);
                }
        if (clientData.pm10!=0){
                    DataStr += QString("PM 10 (ug/m3):  %1\n" ).arg(clientData.pm10);
                }
        if (clientData.CO2!=0){
                    DataStr += QString("CO2【二氧化碳】:  %1\n" ).arg(clientData.CO2);
                }
        if (clientData.VOC!=0){
                    DataStr += QString("VOC【甲醛】:  %1\n" ).arg(clientData.VOC);
                }
        if (clientData.PolarVoltP!=0){
                    DataStr += QString("Polar Voltage Positive【极板正电压（V）】:  %1\n" ).arg(clientData.PolarVoltP);
                }
        if (clientData.PolarVoltN!=0){
                    DataStr += QString("Polar Voltage Negative【极板正电压（V）】:  %1\n" ).arg(clientData.PolarVoltN);
                }
        if (clientData.TubeTempL!=0){
                    DataStr += QString("Tube Temperature Left【左管温度（℃）】:  %1\n" ).arg(clientData.TubeTempL);
                }
        if (clientData.TubeTempR!=0){
                    DataStr += QString("Tube Temperature Right【右管温度（℃）】:  %1\n" ).arg(clientData.TubeTempR);
                }
        if (clientData.RPML!=0){
                    DataStr += QString("Fan Speed Left【左风扇转速（转/s）】:  %1\n" ).arg(clientData.RPML);
                }
        if (clientData.RPMR!=0){
                    DataStr += QString("Fan Speed Right【右风扇转速（转/s）】:  %1\n" ).arg(clientData.RPMR);
                }
        if (clientData.fanOnIonCountN!=0){
                    DataStr += QString("Fan On Negative Ion Count 【开风机负离子采集数】:  %1\n" ).arg(clientData.fanOnIonCountN);
                }
        if (clientData.fanOffIonCountN!=0){
                    DataStr += QString("Fan Off Negative Ion Count 【关风机负离子采集数】:  %1\n" ).arg(clientData.fanOffIonCountN);
                }
        if (clientData.fanOnIonCountP!=0){
                    DataStr += QString("Fan On Positive Ion Count 【开风机正离子采集数】:  %1\n" ).arg(clientData.fanOnIonCountP);
                }
        if (clientData.fanOffIonCountP!=0){
                    DataStr += QString("Fan Off Positive Ion Count 【关风机正离子采集数】:  %1\n" ).arg(clientData.fanOffIonCountP);
                }
        if (clientData.interval!=0){
                    DataStr += QString("Interval【帧标识】:  %1\n" ).arg(clientData.interval);
                }
        if (clientData.elementCount!=0){
                    DataStr += QString("Element Count【观测要素】:  %1\n" ).arg(clientData.elementCount);
                }
        if (clientData.statusCount!=0){
                    DataStr += QString("Status Count【状态要素】:  %1\n" ).arg(clientData.statusCount);
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

void AClient::decodeVersion1Data(const QByteArray &dataArray, ClientData &data)
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
    data.clientDate = QString("%1/%2/%3 %4:%5:%6")
                            .arg(2016)
                            .arg(month)
                            .arg(day)
                            .arg(hour)
                            .arg(minute)
                            .arg(second);

    //convert temperature to floating point number, first argument is higher byte,
    // second argument is lower byte
    data.temperature = convertToDecimal(dataArray.mid(15,1), dataArray.mid(16,1));

    //convert humidity to floating point number, same as temeprature
    data.humidity = convertToDecimal(dataArray.mid(17, 1), dataArray.mid(18,1));

    data.nIon = dataArray.mid(19, 2).toHex().toInt(&ok, 16);
    data.pIon = dataArray.mid(21, 2).toHex().toInt(&ok, 16);

    data.windDirection = dataArray.mid(23,2).toHex().toInt(&ok, 16);

    data.windSpeed = convertToDecimal(dataArray.mid(25,1), dataArray.mid(26,1));

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

    data.rainfall = rain_int + rain_frac;

    //pressure needs three bytes
    data.pressure = convertToDecimal(dataArray.mid(29, 2), dataArray.mid(31,1));

    data.ultraViolet = dataArray.mid(32, 2).toHex().toInt(&ok, 16);

    //TODO:  BCC check sum stuff on byte 34~36,  not implemented yet

    int error = 0;  //TBD
    Q_UNUSED(error);
}

void AClient::decodeVersion2Data(const QByteArray &dataArray, ClientData &data)
{
    bool ok = false;
    decodeVersion1Data(dataArray, data);

    data.oxygen = dataArray.mid(37, 2).toHex().toInt(&ok, 16) / 10;

    data.pm1 = dataArray.mid(39, 2).toHex().toInt(&ok, 16) / 10;

    data.pm25 = dataArray.mid(41, 2).toHex().toInt(&ok, 16) / 10;

    data.pm10 = dataArray.mid(43, 2).toHex().toInt(&ok, 16) / 10;
}

void AClient::decodeVersion3Data(const QByteArray &newData, ClientData &data)
{
    data.stationID = newData.mid(3,5);
    data.latitude = newData.mid(9,6).toInt();
    data.longtitude = newData.mid(16,7).toInt();
    data.altitude = newData.mid(24,5).toInt();
    data.serviceType = newData.mid(30,2).toInt();
    data.deviceType = newData.mid(33,4);
    data.deviceString = newData.mid(38,3);


    QString clientID = data.stationID+data.deviceString;

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
    data.clientDate = QString("%1/%2/%3 %4:%5:%6")
                            .arg(year)
                            .arg(month)
                            .arg(day)
                            .arg(hour)
                            .arg(minute)
                            .arg(second);

    data.interval = newData.mid(57,3).toInt();
    data.elementCount = newData.mid(61,3).toInt();
    data.statusCount = newData.mid(65,2).toInt();


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
        data.nIon = ASA_Str.toInt();
    }
    QString AAA5_Str = "";
    if(indexOfAAA5 != -1) {  //-1 is the case that "AAA5" cannot not be found in the sentence
        AAA5_Str = newData.mid(indexOfAAA5+5, newData.indexOf(",", indexOfAAA5+5)-(indexOfAAA5+5));
        data.temperature = AAA5_Str.toInt() / 10.0;
    }
    QString ADA5_Str = "";
    if(indexOfADA5 != -1) {  //-1 is the case that "ASA" cannot not be found in the sentence
        ADA5_Str = newData.mid(indexOfADA5+5, newData.indexOf(",", indexOfADA5+5)-(indexOfADA5+5));
        data.humidity = ADA5_Str.toInt();
    }
    QString ASB_Str = "";
    if(indexOfASB != -1) {  //-1 is the case that "ASB" cannot not be found in the sentence
        ASB_Str = newData.mid(indexOfASB+4, newData.indexOf(",", indexOfASB+4)-(indexOfASB+4));
        data.PolarVoltN = ASB_Str.toInt() / 10.0;
    }
    QString ASC_Str = "";
    if(indexOfASC != -1) {  //-1 is the case that "ASC" cannot not be found in the sentence
        ASC_Str = newData.mid(indexOfASC+4, newData.indexOf(",", indexOfASC+4)-(indexOfASC+4));
        data.RPML = ASC_Str.toInt();
    }
    QString ASD_Str = "";
    if(indexOfASD != -1) {  //-1 is the case that "ASD" cannot not be found in the sentence
        ASD_Str = newData.mid(indexOfASD+4, newData.indexOf(",", indexOfASD+4)-(indexOfASD+4));
        data.TubeTempL = ASD_Str.toInt() / 10.0;
    }
    QString ASE_Str = "";
    if(indexOfASE != -1) {  //-1 is the case that "ASE" cannot not be found in the sentence
        ASE_Str = newData.mid(indexOfASE+4, newData.indexOf(",", indexOfASE+4)-(indexOfASE+4));
        data.TubeHumidityL = ASE_Str.toInt()/1.0;
    }
    QString ASF_Str = "";
    if(indexOfASF != -1) {  //-1 is the case that "ASF" cannot not be found in the sentence
        ASF_Str = newData.mid(indexOfASF+4, newData.indexOf(",", indexOfASF+4)-(indexOfASF+4));
        data.pressure = ASF_Str.toInt()/10.0;
    }
    QString ASG_Str = "";
    if(indexOfASG != -1) {  //-1 is the case that "ASG" cannot not be found in the sentence
        ASG_Str = newData.mid(indexOfASG+4, newData.indexOf(",", indexOfASG+4)-(indexOfASG+4));
        data.insulation = ADA5_Str.toInt();
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
        data.status = z_Str.toInt();
    }
    QString y_AAA_Str = "";
    if(indexOfy_AAA != -1) {  //-1 is the case that "y_AAA" cannot not be found in the sentence
        y_AAA_Str = newData.mid(indexOfy_AAA+6, newData.indexOf(",", indexOfy_AAA+6)-(indexOfy_AAA+6));
        data.humidity = ADA5_Str.toInt();
    }
    QString y_ADA_Str = "";
    if(indexOfy_ADA != -1) {  //-1 is the case that "y_ADA" cannot not be found in the sentence
        y_ADA_Str = newData.mid(indexOfy_ADA+6, newData.indexOf(",", indexOfy_ADA+6)-(indexOfy_ADA+6));
        data.humidity = ADA5_Str.toInt();
    }
    QString xA_Str = "";
    if(indexOfxA != -1) {  //-1 is the case that "xA" cannot not be found in the sentence
        xA_Str = newData.mid(indexOfxA+3, newData.indexOf(",", indexOfxA+3)-(indexOfxA+3));
        data.humidity = ADA5_Str.toInt();
    }
    QString xB_Str = "";
    if(indexOfxB != -1) {  //-1 is the case that "xB" cannot not be found in the sentence
        xB_Str = newData.mid(indexOfxB+3, newData.indexOf(",", indexOfxB+3)-(indexOfxB+3));
        data.humidity = ADA5_Str.toInt();
    }
    QString wA_Str = "";
    if(indexOfwA != -1) {  //-1 is the case that "wA" cannot not be found in the sentence
        wA_Str = newData.mid(indexOfwA+3, newData.indexOf(",", indexOfwA+3)-(indexOfwA+3));
        data.humidity = ADA5_Str.toInt();
    }

    QString tQ_Str = "";
    if(indexOftQ != -1) {  //-1 is the case that "tQ" cannot not be found in the sentence
        tQ_Str = newData.mid(indexOftQ+7, newData.indexOf(",", indexOftQ+3)-(indexOftQ+3));
        data.humidity = ADA5_Str.toInt();
    }
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

//    int* threadId = (int *)(this->thread()->currentThreadId());
//    m_ThreadId = QString::number(*threadId);

//    if(!QSqlDatabase::contains(m_ThreadId)) {
//        db = QSqlDatabase::addDatabase("QODBC", m_ThreadId);
//    } else {
//        db = QSqlDatabase::database(m_ThreadId);
//    }

    if(!QSqlDatabase::contains(m_ClientId)) {
        db = QSqlDatabase::addDatabase("QODBC", m_ClientId);
    } else {
        db = QSqlDatabase::database(m_ClientId);
    }

    QString dsn = QString("Driver={sql server};server=%1;database=%2;uid=%3;pwd=%4;")

            .arg(settings.readDatabaseSettings("host", "").toString())
            .arg(settings.readDatabaseSettings("DbName", "").toString())
            .arg(settings.readDatabaseSettings("user", "").toString())
            .arg(settings.readDatabaseSettings("password", "").toString());

    db.setDatabaseName(dsn);

    if(db.open()) {
        QString queryStr;
        if(m_ClientVersion == eVersion1) {
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
        } else if (m_ClientVersion == eVersion2) {
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
        } else if (m_ClientVersion == eVersion3) {
            queryStr = QString("INSERT INTO 分钟资料 (区站号, SationID, data_date, data_hour, data_Min, 浓度, 湿度, 温度, 正离子数, 风向, 风速, 雨量, 气压, CO2, PM1, PM25, PM10, 测量室负温度, 测量室正温度, "
                                        "甲醛, 极板负电压, 极板正电压, 风扇负转速, 风扇正转速, 关风机采集数, 开风机采集数, 关风机正离子, 开风机正离子, 经度, 纬度, 海拔高度, 服务类型, 设备标识, 帧标识, 设备标识码)"
                               "VALUES (%1, '%2', '%3', %4, %5, %6, %7, %8, %9, %10, %11, %12, %13, %14, %15, %16, %17, %18, %19, %20, %21, %22, %23, %24, %25, %26, %27, %28, %29, %30, %31, %32, %33, %34);"
                               )
                    .arg(data.stationID)
                    .arg(data.deviceString)
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
                    .arg(data.CO2)
                    .arg(data.pm1)
                    .arg(data.pm25)
                    .arg(data.pm10)
                    .arg(data.TubeTempL)
                    .arg(data.TubeTempR)
                    .arg(data.VOC)
                    .arg(data.PolarVoltN)
                    .arg(data.PolarVoltP)
                    .arg(data.RPML)
                    .arg(data.RPMR)
                    .arg(data.fanOffIonCountN)
                    .arg(data.fanOnIonCountN)
                    .arg(data.fanOffIonCountP)
                    .arg(data.fanOnIonCountP)
                    .arg(data.longtitude)
                    .arg(data.latitude)
                    .arg(data.altitude)
                    .arg(data.serviceType)
                    .arg(data.deviceType)
                    .arg(data.interval)
                    .arg(data.deviceID);
//            queryStr = QString("INSERT INTO 分钟资料 (区站号, SationID, data_date, data_hour, data_Min, 浓度, 湿度, 温度)"
//                               "VALUES (%1, '%2', '%3', %4, %5, %6, %7, %8);"
//                               )
//                    .arg(data.stationID)
//                    .arg(data.deviceString)
//                    .arg(data.clientDate)
//                    .arg(0)
//                    .arg(0)
//                    .arg(data.nIon)
//                    .arg(data.humidity)
//                    .arg(data.temperature);
        }

        QSqlQuery query(db);
        result = query.exec(queryStr);
        if(result==false) {
            qDebug() << "Insert failed\n";
            qDebug() << "stationID: "+data.stationID;
            qDebug() << "deviceString: "+data.deviceString;
            qDebug() << "Date: "+data.clientDate;
            qDebug() << "nIon: "+data.nIon;
            qDebug() << "humidity: "+QString::number(data.humidity);
            qDebug() << "temp: "+QString::number(data.temperature);
            qDebug() << query.lastError().text();
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
