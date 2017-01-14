#pragma once

#include <QTableView>
#include "DataTableModel.h"

class AClient;
class AClientList;

class DataTableView : public QTableView
{
    Q_OBJECT

public:
    DataTableView(AClientList *pClientList, QWidget *pParent = 0);
    ~DataTableView();

private:
    void setupTable();

    DataTableModel *m_pDataTableModel;
    AClientList *m_pClientList;

signals:

private slots:

};
