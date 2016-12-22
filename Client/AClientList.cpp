#include "AClientList.h"
#include <QThread>
#include <algorithm>

AClientList::AClientList(QObject *pParent)
    : QObject(pParent)
{
    m_ClientList.clear();
}

AClientList::~AClientList()
{
    m_ClientList.clear();
}

void AClientList::addClient(AClient *pClient)
{
    //put the next client into the list
    m_ClientList.append(pClient);
    emit clientAdded();
}

void AClientList::removeClient(const int index)
{
    m_ClientList.remove(index);
    emit clientRemoved(index);
}

void AClientList::removeAll()
{
    for(int i=0; i<m_ClientList.size(); i++){
        AClient* pClient = m_ClientList.at(i);
        if(pClient->getClientState()=="Online") {
            //use invokemethod to interact with object in different thread
            QMetaObject::invokeMethod(pClient, "disconnectClient", Qt::BlockingQueuedConnection);
            //delete pClient->thread();
            delete pClient;
        }
    }

    m_ClientList.clear();
}

void AClientList::sort(const int column, Qt::SortOrder order)
{
    qDebug() << "Before sort:\n";
    for(int i=0; i<m_ClientList.size(); i++) {
        qDebug() << m_ClientList.at(i)->getClientId() << endl;
    }

    if(column == 0) {
        if(order == Qt::AscendingOrder)
            std::sort(m_ClientList.begin(), m_ClientList.end(),
                      [](AClient* a, AClient* b) -> bool { return a->getClientId().toInt() < b->getClientId().toInt(); });
        else
            std::sort(m_ClientList.begin(), m_ClientList.end(),
                      [](AClient* a, AClient* b) -> bool { return a->getClientId().toInt() > b->getClientId().toInt(); });
    }

    qDebug() << "After sort:\n";
    for(int i=0; i<m_ClientList.size(); i++) {
        qDebug() << m_ClientList.at(i)->getClientId() << endl;
    }
}

int AClientList::size() const
{
    return m_ClientList.size();
}

AClient* AClientList::getClient(const int index)
{
    return m_ClientList[index];
}

void AClientList::onClientDataChanged()
{
    AClient* pClient = static_cast<AClient*>(QObject::sender());
    for(int i=0; i<m_ClientList.size(); i++) {
        if(m_ClientList.at(i)->getClientId() == pClient->getClientId()) {
            emit clientDataChanged(i);
            break;
        }
    }
}
