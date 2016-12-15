#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSettings>

class AppSettings : public QSettings
{
    Q_OBJECT

public:
    explicit AppSettings(QObject *parent = 0);

    void writeSettings(const QString &group, const QString &key, const QVariant &value);
    QVariant readSettings(const QString &group, const QString &key, const QVariant &defaultValue);

    void writeDatabaseSettings(const QString &key, const QVariant &value);
    QVariant readDatabaseSettings(const QString &key, const QVariant &defaultValue);

    void writeServerSettings(const QString &key, const QVariant &value);
    QVariant readServerSettings(const QString &key, const QVariant &defaultValue);

    void writeMiscSettings(const QString &key, const QVariant &value);
    QVariant readMiscSettings(const QString &key, const QVariant &defaultValue);

    void setDefaultValues();

signals:

public slots:
};

#endif // APPSETTINGS_H
