#include "ClientTableWidget.h"
#include "TheServer.h"
#include "AClient.h"
#include "DataViewer.h"
#include "ClientCommandDialog.h"
#include "Misc/Logger.h"
#include <QHeaderView>
#include <QTimer>
#include <QAction>
#include <QInputEvent>
#include <QMenu>

#define UPDATE_INTERVAL 1000

ClientTableWidget::ClientTableWidget(TheServer *pServer, QWidget *pParent)
    : QTableWidget(pParent),
      m_pServer(pServer)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(showContextMenu(QPoint)));

    setupTable();

    //set up how often the table update
    m_pUpdateTimer = new QTimer(this);
    m_pUpdateTimer->setInterval(UPDATE_INTERVAL);
    connect(m_pUpdateTimer, SIGNAL(timeout()), this, SLOT(updateTable()));
    m_pUpdateTimer->start();
}

ClientTableWidget::~ClientTableWidget()
{

}

void ClientTableWidget::setupTable()
{
    QStringList tableHeaders;
    tableHeaders <<QString(tr("ID"))<<QString(tr("Status"))<<QString(tr("Source"))<<QString(tr("Time Online")) << QString(tr("Time Offline")) << QString(tr("Up Time"));

    setColumnCount(tableHeaders.count());
    setHorizontalHeaderLabels(tableHeaders);
    //this->verticalHeader()->setVisible(false);

    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
}


void ClientTableWidget::updateTable()
{
    setRowCount(m_pServer->getTotalClient());

    //List out the clients one by one in a loop
    for(int i=0; i<m_pServer->getTotalClient(); i++) {
        AClient *pClient = m_pServer->getClient(i);
        if(pClient == NULL) return; //make sure the client exist

        //draw all the texts
        this->setItem(i, 0, new QTableWidgetItem(QString::number(pClient->getClientId())));

        QString state = pClient->getClientState();
        this->setItem(i, 1, new QTableWidgetItem(state));
        if(state == "Offline")
            item(i, 1)->setBackgroundColor(QColor(255, 0, 0));
        else if(state == "No Data")
            item(i, 1)->setBackgroundColor(QColor(245, 245, 100));
        else
            item(i, 1)->setBackgroundColor(QColor(0, 240, 100));

        this->setItem(i, 2, new QTableWidgetItem(pClient->getClientAddress()));
        this->setItem(i, 3, new QTableWidgetItem(pClient->getClientConnectTime()
                                                 .toString(QString("yyyy/MM/dd hh:mm:ss"))));
        this->setItem(i, 4, new QTableWidgetItem(pClient->getClientDisconnectTime()));
        this->setItem(i, 5, new QTableWidgetItem(pClient->getClientUpTime()));
    }
}

//deselect selection when left click
void ClientTableWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        QModelIndex item = indexAt(event->pos());
        bool selected = selectionModel()->isSelected(indexAt(event->pos()));
        QTableWidget::mousePressEvent(event);
        if ((item.row() == -1 && item.column() == -1) || selected)
        {
            clearSelection();
        }
    }
    else
        QTableWidget::mousePressEvent(event);
}

//this will show he right click pop-up menu
void ClientTableWidget::showContextMenu(const QPoint& pos) // this is a slot
{
    Q_UNUSED(pos);
    //make sure there is someting to be selected
    if(rowCount() != 0 && !selectedItems().isEmpty()) {
        QMenu *pMenu = new QMenu(this);

        //send command dialog action
        QAction *pSendCommandAction = new QAction(QString(tr("Send Command...")), pMenu);
        connect(pSendCommandAction, SIGNAL(triggered()), this, SLOT(onSendCommandTriggered()));
        pMenu->addAction(pSendCommandAction);

        //open data viewer action
        QAction *pMsgViewAction = new QAction(QString(tr("View Data...")), pMenu);
        pMsgViewAction->setCheckable(true);
        if(m_pServer->getClient(currentRow())->getDataViewer() != NULL){
            pMsgViewAction->setChecked(true);
        }
//      pMsgViewAction->setChecked(m_pServer->getClient(currentRow())->getDataViewer());
        connect(pMsgViewAction, SIGNAL(triggered()), this, SLOT(onMessageViewerTriggered()));
        pMenu->addAction(pMsgViewAction);

        //add action to toggle show client in chart dialog
        QAction *pShowChartAction = new QAction(QString(tr("Show Chart")), pMenu);
        pShowChartAction->setCheckable(true);
        pShowChartAction->setChecked(m_pServer->getClient(currentRow())->getShowChart());
        connect(pShowChartAction, SIGNAL(toggled(bool)), this, SLOT(onShowChartToggled(bool)));
        pMenu->addAction(pShowChartAction);

        pMenu->exec(QCursor::pos());
    }
}

//open a dialog to send command to client
void ClientTableWidget::onSendCommandTriggered()
{
    //modaless
    ClientCommandDialog dialog(m_pServer->getClient(currentRow()), this);
    dialog.exec();
}

//Open a message viewer dialg when option selected
void ClientTableWidget::onMessageViewerTriggered()
{
    //use this dialog to show client data

    //modaless
    //DataViewer viewer(m_pServer->getClient(currentRow()), this);
    //viewer.exec();

    //modal
    AClient* pSelectedClient = m_pServer->getClient(currentRow());
    if(pSelectedClient->getDataViewer() == NULL) {
        DataViewer *pViewer = new DataViewer(pSelectedClient, this);
        pViewer->setAttribute( Qt::WA_DeleteOnClose, true);
        pViewer->show();
    } else {
        pSelectedClient->getDataViewer()->show();
    }
}

//This tells the GUI to add this client to the chart dialog
void ClientTableWidget::onShowChartToggled(const bool enabled)
{
    AClient *pClient = m_pServer->getClient(currentRow());
    pClient->setShowChart(enabled);
    emit showChart(enabled, pClient);
}
