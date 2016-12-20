#pragma once

#include <QAbstractTableModel>
#include <AClient.h>

class AClientList;

class ClientTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ClientTableModel(QObject *pParent = 0);
    ~ClientTableModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    void setClientList(AClientList *pClientList);

private:
    //How often we update the table
    QTimer *m_pUpdateTimer;
    AClientList *m_pClientList;

private slots:
    void onUpdateTimer();
    void onNewClientAdded();
    void onClientRemoved(const int index);
};
