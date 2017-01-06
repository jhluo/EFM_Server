#ifndef OFFSETSETTINGS_H
#define OFFSETSETTINGS_H

#include <QSettings>

class OffsetSettings : public QSettings
{
    Q_OBJECT

public:
    explicit OffsetSettings(QObject *parent = 0);

    void writeBaseOffset(const QString &id, const QVariant &value);
    void writeMultiplierOffset(const QString &id, const QVariant &value);

    double readBaseOffset(const QString &id);
    double readMultiplierOffset(const QString &id);

    void setDefaultValues();

signals:

public slots:
};

#endif // OFFSETSETTINGS_H
