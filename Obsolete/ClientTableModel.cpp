#include "ClientTableModel.h"

#define UPDATE_INTERVAL 1000

ClientTableModel::ClientTableModel(QObject *pParent)
    : QAbstractTableModel(pParent)
{
    //set up how often the table update
    m_pUpdateTimer = new QTimer(this);
    m_pUpdateTimer->setInterval(UPDATE_INTERVAL);
    connect(m_pUpdateTimer, SIGNAL(timeout()), this, SLOT(updateModel()));
    m_pUpdateTimer->start();
}

ClientTableModel::~ClientTableModel()
{

}

void ClientTableModel::setClientList(QList<AClient*> *pClientList)
{
    m_pClientList = pClientList;
}

int ClientTableModel::rowCount(const QModelIndex & /*parent*/) const
{
   return m_pClientList->size();
}

int ClientTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 5;
}

QVariant ClientTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("Client ID");
            case 1:
                return QString("Status");
            case 2:
                return QString("Time Online");
            case 3:
                return QString("Time Offline");
            case 4:
                return QString("Up Time");
            }
        }
    }
    return QVariant();
}

QVariant ClientTableModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    if (role == Qt::DisplayRole)
    {
        for (int i=0; i<m_pClientList->size(); i++) {
            if(row == i) {
                if(col == 0) {
                    return m_pClientList->at(i)->getClientId();
                } else if (col == 1) {
                    return m_pClientList->at(i)->getClientState();
                } else if (col == 2) {
                    return m_pClientList->at(i)->getClientConnectTime();
                } else if (col == 3) {
                    return m_pClientList->at(i)->getClientDisconnectTime();
                } else if (col == 4) {
                    return m_pClientList->at(i)->getClientUpTime();
                }
            }
        }
    }

    return QVariant();
}

void ClientTableModel::updateModel()
{
    //we identify the top left cell
    QModelIndex topLeft = createIndex(0,0);
    QModelIndex bottomRight = createIndex(rowCount()-1, columnCount()-1);
    //emit a signal to make the view reread identified data
    emit dataChanged(topLeft, bottomRight);
}
