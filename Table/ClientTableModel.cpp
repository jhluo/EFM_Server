#include "ClientTableModel.h"
#include "TheServer.h"

#define UPDATE_INTERVAL 1000

ClientTableModel::ClientTableModel(TheServer *pServer, QObject *pParent)
    : QAbstractTableModel(pParent),
      m_pServer(pServer)
{
    m_Keys = m_pServer->getClientList();

    connect(m_pServer, SIGNAL(clientAdded(QString)), this, SLOT(onNewClientAdded(QString)), Qt::QueuedConnection);
    connect(m_pServer, SIGNAL(clientRemoved(QString)), this, SLOT(onClientRemoved(QString)), Qt::QueuedConnection);
    connect(m_pServer, SIGNAL(clientDataChanged(QString)), this, SLOT(onClientDataUpdated(QString)), Qt::QueuedConnection);

    //set up how often the table update
    m_pUpdateTimer = new QTimer(this);
    m_pUpdateTimer->setInterval(UPDATE_INTERVAL);
    connect(m_pUpdateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimer()));
    //m_pUpdateTimer->start();
}

ClientTableModel::~ClientTableModel()
{

}


int ClientTableModel::rowCount(const QModelIndex & /*parent*/) const
{
   return m_Keys.size();
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
    int col = index.column();

    AClient *pClient = m_pServer->getClient(m_Keys.at(index.row()));

    if(pClient==NULL) return QVariant();

    if (role == Qt::DisplayRole)
    {
        if(col == 0) return pClient->getClientId();
        if(col == 1) return pClient->getClientAddress();
        if(col == 2) {
            if(pClient->getClientState()==AClient::eOffline)
                return "Offline";
            else if(pClient->getClientState()==AClient::eNoData)
                return "No Data";
            else if(pClient->getClientState()==AClient::eUnknownState)
                return "Pending...";
            else
                return "Online";
        }
        if(col == 3) return pClient->getClientConnectTime();
        if(col == 4) return pClient->getClientDisconnectTime();
        if(col == 5) return pClient->getClientUpTime();
    }

    if(role == Qt::BackgroundRole)
    {
        if (col==2) {
            AClient::eClientState state = pClient->getClientState();
            if(state == AClient::eOffline) {
                QBrush background(QColor(255, 0, 0));
                return background;
            } else if(state == AClient::eNoData) {
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
    //m_pServer->sortClients(column, order);

    //refresh the entire table
    QModelIndex topLeft = createIndex(0,0);
    QModelIndex bottomRight = createIndex(rowCount()-1, columnCount()-1);
    //emit a signal to make the view reread identified data
    emit dataChanged(topLeft, bottomRight);
}

int ClientTableModel::indexOf(const QString &key) const
{
    return m_Keys.indexOf(key);
}

QString ClientTableModel::keyAt(const int index) const
{
    return m_Keys.at(index);
}

void ClientTableModel::onUpdateTimer()
{
    //we identify the top left cell
    QModelIndex topLeft = createIndex(0,columnCount()-1);
    QModelIndex bottomRight = createIndex(rowCount()-1, columnCount()-1);
    //emit a signal to make the view reread identified data
    emit dataChanged(topLeft, bottomRight);
}

void ClientTableModel::onNewClientAdded(const QString &key)
{
    m_Keys.append(key);
    //insert a row at the end
    int row = rowCount();
    beginInsertRows(QModelIndex(), row, row);
    insertRow(row);
    endInsertRows();

    QModelIndex topLeft = createIndex(rowCount()-1, 0);
    QModelIndex bottomRight = createIndex(rowCount()-1, columnCount()-1);
    emit dataChanged(topLeft, bottomRight);
}

void ClientTableModel::onClientRemoved(const QString &key)
{
    int index = m_Keys.indexOf(key);
    m_Keys.removeAt(index);
    beginRemoveRows(QModelIndex(), index, index);
    removeRow(index);
    endRemoveRows();

    QModelIndex topLeft = createIndex(index, 0);
    QModelIndex bottomRight = createIndex(index, columnCount()-1);
    emit dataChanged(topLeft, bottomRight);
}

void ClientTableModel::onClientDataUpdated(const QString &key)
{
    //this one only updates timer
    int index = m_Keys.indexOf(key);

    //we identify the top left cell
    QModelIndex topLeft = createIndex(index,0);
    QModelIndex bottomRight = createIndex(index, columnCount()-2);
    //emit a signal to make the view reread identified data
    emit dataChanged(topLeft, bottomRight);
}
