#pragma once

#include <QObject>
#include <QVariant>
#include <QTimer>

class tData : public QObject
{
    Q_OBJECT

public:
    explicit tData(QObject *pParent=0);
    tData(const QVariant &value, QObject *pParent=0);

    //copy constructor
    tData(const tData &data);

    ~tData();

    void setValue(const QVariant &value);

    QVariant value() const {return m_DataValue;}

    //overloaded to check two clients as equivalent
    inline void operator=(const tData &rhs){
        m_DataValue = rhs.value();
        m_pDataTimer->stop();
        m_pDataTimer->start();
    }

private:
    QVariant m_DataValue;
    QTimer *m_pDataTimer;

private slots:
    void onDataTimeout();
};

