#include "AppSettings.h"

#include <QDir>

AppSettings::AppSettings(QObject *parent)
    :QSettings(QSettings::IniFormat,
               QSettings::UserScope,
               "EPEX", "IonServer", parent)
{
    setDefaultValues();
}

void AppSettings::writeMiscSettings(const QString &key, const QVariant &value)
{
    this->beginGroup("Misc");
    this->setValue(key, value);
    this->endGroup();
}

QVariant AppSettings::readMiscSettings(const QString &key, const QVariant &defaultValue)
{
    QVariant value;
    this->beginGroup("Misc");
    value = this->value(key, defaultValue);
    this->endGroup();

    return value;
}

void AppSettings::writeServerSettings(const QString &key, const QVariant &value)
{
    this->beginGroup("Server");
    this->setValue(key, value);
    this->endGroup();
}

QVariant AppSettings::readServerSettings(const QString &key, const QVariant &defaultValue)
{
    QVariant value;
    this->beginGroup("Server");
    value = this->value(key, defaultValue);
    this->endGroup();

    return value;
}

void AppSettings::writeDatabaseSettings(const QString &key, const QVariant &value)
{
    this->beginGroup("Database");
    this->setValue(key, value);
    this->endGroup();
}

QVariant AppSettings::readDatabaseSettings(const QString &key, const QVariant &defaultValue)
{
    QVariant value;
    this->beginGroup("Database");
    value = this->value(key, defaultValue);
    this->endGroup();

    return value;
}

void AppSettings::writeSettings(const QString &group, const QString &key, const QVariant &value)
{
    this->beginGroup(group);
    this->setValue(key, value);
    this->endGroup();
}

QVariant AppSettings::readSettings(const QString &group, const QString &key, const QVariant &defaultValue)
{
    QVariant value;
    this->beginGroup(group);
    value = this->value(key, defaultValue);
    this->endGroup();

    return value;
}

void AppSettings::setDefaultValues()
{
    //create a default configuration
}
