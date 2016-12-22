#include "ClientTableModel.h"
#include "Client/AClientList.h"

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
    connect(m_pClientList, SIGNAL(clientDataChanged(int)), this, SLOT(onClientDataUpdated(int)));
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

        if(orientation==Qt::Vertical)
            return QString::number(section+1);
    }

    return QVariant();
}

QVariant ClientTableModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    if (role == Qt::DisplayRole)
    {
        if(col == 0) return m_pClientList->getClient(row)->getClientId();
        if(col == 1) return m_pClientList->getClient(row)->getClientAddress();
        if(col == 2) return m_pClientList->getClient(row)->getClientState();
        if(col == 3) return m_pClientList->getClient(row)->getClientConnectTime();
        if(col == 4) return m_pClientList->getClient(row)->getClientDisconnectTime();
        if(col == 5) return m_pClientList->getClient(row)->getClientUpTime();
    }

    if(role == Qt::BackgroundRole)
    {
        if (col==2) {
            QString state = m_pClientList->getClient(row)->getClientState();
            if(state == "Offline") {
                QBrush background(QColor(255, 0, 0));
                return background;
            } else if(state == "No Data") {
                QBrush background(QColor(245, 245, 100));
                return background;
            } else {
                QBrush background(QColor(0, 240, 100));
                return background;
            }
        }
    }

    return QVariant();
}

void ClientTableModel::sort(int column, Qt::SortOrder order)
{
    m_pClientList->sort(column, order);

    //refresh the entire table
    QModelIndex topLeft = createIndex(0,0);
    QModelIndex bottomRight = createIndex(rowCount()-1, columnCount()-1);
    //emit a signal to make the view reread identified data
    emit dataChanged(topLeft, bottomRight);
}

void ClientTableModel::onUpdateTimer()
{
    //we identify the top left cell
    QModelIndex topLeft = createIndex(0,columnCount()-1);
    QModelIndex bottomRight = createIndex(rowCount()-1, columnCount()-1);
    //emit a signal to make the view reread identified data
    emit dataChanged(topLeft, bottomRight);
}

void ClientTableModel::onNewClientAdded()
{
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
    //insert a row at the end
    beginRemoveRows(QModelIndex(), index, index);
    removeRow(index);
    endRemoveRows();

    QModelIndex topLeft = createIndex(index, 0);
    QModelIndex bottomRight = createIndex(index, columnCount()-1);
    emit dataChanged(topLeft, bottomRight);
}

void ClientTableModel::onClientDataUpdated(const int index)
{
    //we identify the top left cell
    QModelIndex topLeft = createIndex(index,0);
    QModelIndex bottomRight = createIndex(index, columnCount()-2);
    //emit a signal to make the view reread identified data
    emit dataChanged(topLeft, bottomRight);
}
