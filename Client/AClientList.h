#pragma once

#include <QObject>
#include <QVector>
#include "AClient.h"

//container object of all clients

class AClient;

class AClientList : public QObject
{
    Q_OBJECT

public:
    AClientList(QObject *pParent = 0);
    ~AClientList();

    void addClient(AClient *pClient);
    void removeClient(const int index);

    //remove all instance of this client
    void removeAClient(AClient *pClient);

    void removeAll();
    void sort(const int column, Qt::SortOrder order);

    int size() const;
    AClient* getClient(const int index);

private:
    QVector<AClient*> m_ClientList;

signals:
    void clientAdded();
    void clientRemoved(const int index);
    void clientDataChanged(const int index);

public slots:
    void onClientDataChanged();
};
