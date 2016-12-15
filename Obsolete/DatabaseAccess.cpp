#include "DatabaseAccess.h"
#include "AppSettings.h"

DatabaseAccess::DatabaseAccess(QObject *pParent):
    QObject(pParent),
    m_ConnectionName("")
{

}

DatabaseAccess::~DatabaseAccess()
{

}

bool DatabaseAccess::connectToDB(const QString &name)
{
    AppSettings settings;

    m_ConnectionName = name;

    QSqlDatabase db;
    if (!db.contains(name)) {
        db = QSqlDatabase::addDatabase("QODBC", m_ConnectionName);
    } else {
        db = QSqlDatabase::database(name);
    }

    QString dsn = QString("Driver={sql server};server=%1;database=%2;uid=%3;pwd=%4;")
            .arg(settings.readDatabaseSettings("host", "").toString())
            .arg(settings.readDatabaseSettings("DbName", "").toString())
            .arg(settings.readDatabaseSettings("user", "").toString())
            .arg(settings.readDatabaseSettings("password", "").toString());

    db.setDatabaseName(dsn);

    if(db.open()) {
        return true;
    }

    qDebug() << QString("Client %1 database connection error: ").arg(m_ConnectionName) << db.lastError().text();
    return false;
}

bool DatabaseAccess::writeData(const int clientId, const QString &date, const double temperature,
                               const double humidity, const int nIon, const int pIon, const int windD,
                               const double windS, const double rain, const double pressure,
                               const double ultraviolet, const int oxygen, const int PM1, const int PM25,
                               const int PM10, const int error)
{
    QSqlDatabase db = QSqlDatabase::database(m_ConnectionName);
    bool result = false;

    if (db.open()) {
//        QSqlQuery query(db);
//        query.prepare("INSERT INTO 分钟资料 (SationID, data_date, data_hour, data_Min, 浓度, 湿度, 温度, 正离子数, 风向, 风速, 雨量, 气压, 紫外线, 氧气含量, PM1, PM25, PM10, 错误标志) "
//                      "VALUES (:SationID, :data_date, :data_hour, :data_Min, :浓度, :湿度, :温度, :正离子数, :风向, :风速, :雨量, :气压, :紫外线, :氧气含量, :PM1, :PM25, :PM10, :错误标志);");
//        query.bindValue(":SationID", clientId);
//        query.bindValue(":data_date", date);
//        query.bindValue(":data_hour", 0);
//        query.bindValue(":data_Min", 0);
//        query.bindValue(":浓度", nIon);
//        query.bindValue(":湿度", humidity);
//        query.bindValue(":温度", temperature);
//        query.bindValue(":正离子数", pIon);
//        query.bindValue(":风向", windD);
//        query.bindValue(":风速", windS);
//        query.bindValue(":雨量", rain);
//        query.bindValue(":气压", pressure);
//        query.bindValue(":紫外线", ultraviolet);
//        query.bindValue(":氧气含量", oxygen);
//        query.bindValue(":PM1", PM1);
//        query.bindValue(":PM25", PM25);
//        query.bindValue(":PM10", PM10);
//        query.bindValue(":错误标志", error);
//        result = query.exec();

        QString queryStr;
        queryStr = QString("INSERT INTO 分钟资料 (SationID, data_date, data_hour, data_Min, 浓度, 湿度, 温度, 正离子数, 风向, 风速, 雨量, 气压, 紫外线, 氧气含量, PM1, PM25, PM10, 错误标志)"
                           "VALUES ('%1', '%2', %3, %4, %5, %6, %7, %8, %9, %10, %11, %12, %13, %14, %15, %16, %17, %18);"
                           )
                .arg(clientId)
                .arg(date)
                .arg(0)
                .arg(0)
                .arg(nIon)
                .arg(humidity)
                .arg(temperature)
                .arg(pIon)
                .arg(windD)
                .arg(windS)
                .arg(rain)
                .arg(pressure)
                .arg(ultraviolet)
                .arg(oxygen)
                .arg(PM1)
                .arg(PM25)
                .arg(PM10)
                .arg(error);

        qDebug() << queryStr;
        QSqlQuery query = db.exec(queryStr);
        result = query.isValid();
    } else {
        qDebug() << QString("Client %1 failed to open database when executing query.").arg(m_ConnectionName) << db.lastError().text();
    }


    return result;
}
