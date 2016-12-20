#include "ClientTableModel.h"
#include "AClientList.h"

#define UPDATE_INTERVAL 1000

ClientTableModel::ClientTableModel(QObject *pParent)
    : QAbstractTableModel(pParent)
{
    //set up how often the table update
    m_pUpdateTimer = new QTimer(this);
    m_pUpdateTimer->setInterval(UPDATE_INTERVAL);
    connect(m_pUpdateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimer()));
    m_pUpdateTimer->start();
}

ClientTableModel::~ClientTableModel()
{

}

void ClientTableModel::setClientList(AClientList *pClientList)
{
    m_pClientList = pClientList;
    connect(m_pClientList, SIGNAL(clientAdded()), this, SLOT(onNewClientAdded()));
    connect(m_pClientList, SIGNAL(clientRemoved(int)), this, SLOT(onClientRemoved(int)));
}

int ClientTableModel::rowCount(const QModelIndex & /*parent*/) const
{
   return m_pClientList->size();
}

int ClientTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 6;
}

QVariant ClientTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
                case 0:
                    return QString(tr("Client ID"));
                case 1:
                    return QString(tr("Source"));
                case 2:
                    return QString(tr("Status"));
                case 3:
                    return QString(tr("Time Online"));
                case 4:
                    return QString(tr("Time Offline"));
                case 5:
                    return QString(tr("Up Time"));
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
                    return m_pClientList->getClient(i)->getClientId();
                } else if (col == 1) {
                    return m_pClientList->getClient(i)->getClientAddress();
                } else if (col == 2) {
                    return m_pClientList->getClient(i)->getClientState();
                } else if (col == 3) {
                    return m_pClientList->getClient(i)->getClientConnectTime();
                } else if (col == 4) {
                    return m_pClientList->getClient(i)->getClientDisconnectTime();
                } else if (col == 5) {
                    return m_pClientList->getClient(i)->getClientUpTime();
                }
            }
        }
    }

    return QVariant();
}

void ClientTableModel::onUpdateTimer()
{
    //we identify the top left cell
    QModelIndex topLeft = createIndex(0,0);
    QModelIndex bottomRight = createIndex(rowCount()-1, columnCount()-1);
    //emit a signal to make the view reread identified data
    emit dataChanged(topLeft, bottomRight);
}

void ClientTableModel::onNewClientAdded()
{
    //int row = m_data.count();
    //beginInsertRows(QModelIndex(), row, row);
    //m_data.append(tableData);
    //endInsertRows();

    //emit dataChanged(begin, end);   // emitting dataChanged signal --> blank rows still getting added
    //insert a row at the end
    int row = rowCount();
    beginInsertRows(QModelIndex(), row, row);
    insertRow(row);
    endInsertRows();

    QModelIndex topLeft = createIndex(rowCount()-1, 0);
    QModelIndex bottomRight = createIndex(rowCount()-1, columnCount()-1);
    emit dataChanged(topLeft, bottomRight);
}

void ClientTableModel::onClientRemoved(const int index)
{

}
