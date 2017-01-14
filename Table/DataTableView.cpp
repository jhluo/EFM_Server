#include "DataTableView.h"
#include "TheServer.h"
#include "Client/AClient.h"
#include "Client/AClientList.h"

DataTableView::DataTableView(AClientList *pClientList, QWidget *pParent)
    : QTableView(pParent),
      m_pClientList(pClientList)
{
    setupTable();

    m_pDataTableModel = new DataTableModel(this);

    //this add sorting
    //QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    //proxyModel->setSourceModel(m_pClientTableModel);
    //setModel(proxyModel);

    setModel(m_pDataTableModel);
}

DataTableView::~DataTableView()
{

}

void DataTableView::setupTable()
{

}
