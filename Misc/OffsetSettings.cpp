#include "OffsetSettings.h"

#include <QDir>

OffsetSettings::OffsetSettings(QObject *parent)
    :QSettings(QString("localOffset"),
               QSettings::IniFormat,
               parent)
{
    setDefaultValues();
}

void OffsetSettings::writeBaseOffset(const QString &id, const QVariant &value)
{
    this->beginGroup("offset");
    this->setValue(id+"/base", value);
    this->endGroup();
}

void OffsetSettings::writeMultiplierOffset(const QString &id, const QVariant &value)
{
    this->beginGroup("offset");
    this->setValue(id+"/multiplier", value);
    this->endGroup();
}

double OffsetSettings::readBaseOffset(const QString &id)
{
    QVariant value;

    this->beginGroup("offset");

//    QStringList keys = childKeys();
//    foreach(QString key, keys) {
//        if(key == id) {
//            value = this->value(id+"/base", 0.0);
//        }
//    }

    value = this->value(id+"/base", 0.0);
    this->endGroup();

    return value.toDouble();
}

double OffsetSettings::readMultiplierOffset(const QString &id)
{
    QVariant value;

    this->beginGroup("offset");

    value = this->value(id+"/multiplier", 1.0);
    this->endGroup();

    return value.toDouble();
}

void OffsetSettings::setDefaultValues()
{
    //create a default configuration
}
