#pragma once

#include <QAbstractTableModel>
#include <Client/AClient.h>

class TheServer;

class ClientTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ClientTableModel(TheServer *pServer, QObject *pParent = 0);
    ~ClientTableModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    int indexOf(const QString &key) const;
    QString keyAt(const int index) const;

private:
    //How often we update the table
    QTimer *m_pUpdateTimer;
    TheServer *m_pServer;
    QStringList m_Keys;

private slots:
    void onUpdateTimer();
    void onNewClientAdded(const QString &key);
    void onClientRemoved(const QString &key);
    void onClientDataUpdated(const QString &key);
};
