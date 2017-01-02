#pragma once

#include <QObject>
#include <QVariant>
#include <QTimer>

class tData : public QObject
{
    Q_OBJECT

public:
    explicit tData(QObject *pParent=0);
    tData(const tData &data);
    ~tData();

    void setValue(const QVariant &value);

    QVariant value() const {return m_DataValue;}

private:
    QVariant m_DataValue;
    QTimer m_DataTimer;

private slots:
    void onDataTimeout();
};

