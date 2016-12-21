#include "AClient.h"
#include "AClientList.h"
#include <QThread>

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
            pClient->closeClient();
            delete pClient->thread();
            delete pClient;
        }
    }

    m_ClientList.clear();
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