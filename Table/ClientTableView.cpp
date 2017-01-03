#include "ClientTableView.h"
#include "TheServer.h"
#include "Client/AClient.h"
#include "Client/AClientList.h"
#include "Widgets/DataViewer.h"
#include "Widgets/ClientCommandDialog.h"
#include "Widgets/SerialSettingsDialog.h"
#include "Misc/Logger.h"
#include <QHeaderView>
#include <QTimer>
#include <QAction>
#include <QInputEvent>
#include <QMenu>

ClientTableView::ClientTableView(AClientList *pClientList, QWidget *pParent)
    : QTableView(pParent),
      m_pClientList(pClientList)
{
    setupTable();

    m_pClientTableModel = new ClientTableModel(this);
    m_pClientTableModel->setClientList(pClientList);

    //this add sorting
    //QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    //proxyModel->setSourceModel(m_pClientTableModel);
    //setModel(proxyModel);

    setModel(m_pClientTableModel);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(showContextMenu(QPoint)));
}

ClientTableView::~ClientTableView()
{

}

void ClientTableView::setupTable()
{
    //this->verticalHeader()->setVisible(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSortingEnabled(true);
}

//deselect selection when left click
void ClientTableView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        QModelIndex item = indexAt(event->pos());
        bool selected = selectionModel()->isSelected(indexAt(event->pos()));
        QTableView::mousePressEvent(event);
        if ((item.row() == -1 && item.column() == -1) || selected)
        {
            clearSelection();
        }
    }
    else
        QTableView::mousePressEvent(event);
}

//this will show he right click pop-up menu
void ClientTableView::showContextMenu(const QPoint& pos) // this is a slot
{
    Q_UNUSED(pos);
    //make sure there is someting to be selected
    if(this->model()->rowCount() != 0 && !selectedIndexes().isEmpty()) {
        QMenu *pMenu = new QMenu(this);
        AClient *pCurrentClient = m_pClientList->getClient(selectedIndexes().first().row());

        //add a connect/disconnect option for serial client
        if(pCurrentClient->getClientType()==AClient::eSerial) {
            QAction *pConnectAction = new QAction(pMenu);
            if(pCurrentClient->getClientState()=="Offline") pConnectAction->setText(tr("Connect"));
            else pConnectAction->setText(tr("Disconnect"));
            connect(pConnectAction, SIGNAL(triggered()), this, SLOT(onSerialConnectTriggered()));
            pMenu->addAction(pConnectAction);

            QAction *pEditAction = new QAction(tr("Edit"), pMenu);
            if(pCurrentClient->getClientState()=="Offline") pEditAction->setEnabled(true);
            else pEditAction->setEnabled(false);
            connect(pEditAction, SIGNAL(triggered()), this, SLOT(onSerialEditTriggered()));
            pMenu->addAction(pEditAction);
        }

        //send command dialog action
        QAction *pSendCommandAction = new QAction(QString(tr("Send Command...")), pMenu);
        connect(pSendCommandAction, SIGNAL(triggered()), this, SLOT(onSendCommandTriggered()));
        pMenu->addAction(pSendCommandAction);

        //open data viewer action
        QAction *pMsgViewAction = new QAction(QString(tr("View Data...")), pMenu);
        pMsgViewAction->setCheckable(true);
        pMsgViewAction->setChecked(m_pClientList->getClient(selectedIndexes().first().row())->getDataViewer() != NULL);
        connect(pMsgViewAction, SIGNAL(toggled(bool)), this, SLOT(onMessageViewerToggled(bool)));
        pMenu->addAction(pMsgViewAction);

        //add action to toggle show client in chart dialog
        QAction *pShowChartAction = new QAction(QString(tr("Show Chart")), pMenu);
        pShowChartAction->setCheckable(true);
        pShowChartAction->setChecked(m_pClientList->getClient(selectedIndexes().first().row())->getShowChart());
        connect(pShowChartAction, SIGNAL(toggled(bool)), this, SLOT(onShowChartToggled(bool)));
        pMenu->addAction(pShowChartAction);

        pMenu->exec(QCursor::pos());
    }
}

//open a dialog to send command to client
void ClientTableView::onSendCommandTriggered()
{
    //modaless
    ClientCommandDialog dialog(m_pClientList->getClient(selectedIndexes().first().row()), this);
    dialog.exec();
}

//Open a message viewer dialg when option selected
void ClientTableView::onMessageViewerToggled(const bool enabled)
{
    //use this dialog to show client data

    //modaless
    //DataViewer viewer(m_pServer->getClient(currentRow()), this);
    //viewer.exec();

    //modal
    AClient* pSelectedClient = m_pClientList->getClient(selectedIndexes().first().row());

    if(enabled) {
        if(pSelectedClient->getDataViewer() == NULL) {
            DataViewer *pViewer = new DataViewer(pSelectedClient, this);
            pViewer->setAttribute( Qt::WA_DeleteOnClose, true);
            pViewer->show();
        } else {
            qobject_cast<QWidget*>(pSelectedClient->getDataViewer()->parent())->show();
        }
    } else {
        if(pSelectedClient->getDataViewer() != NULL) {
            qobject_cast<QWidget*>(pSelectedClient->getDataViewer()->parent())->close();
        }
    }
}

//This tells the GUI to add this client to the chart dialog
void ClientTableView::onShowChartToggled(const bool enabled)
{
    AClient *pClient = m_pClientList->getClient(selectedIndexes().first().row());
    pClient->setShowChart(enabled);
    emit showChart(enabled, pClient);
}

//for connecting serial client
void ClientTableView::onSerialConnectTriggered()
{
    AClient *pCurrentClient = m_pClientList->getClient(selectedIndexes().first().row());
    if(pCurrentClient->getClientState()=="Offline") {
        QMetaObject::invokeMethod(pCurrentClient, "connectClient", Qt::QueuedConnection);
    } else {
        QMetaObject::invokeMethod(pCurrentClient, "disconnectClient", Qt::QueuedConnection);
    }
}

void ClientTableView::onSerialEditTriggered()
{
    AClient *pCurrentClient = m_pClientList->getClient(selectedIndexes().first().row());
    SerialSettingsDialog dialog(qobject_cast<QSerialPort*>(pCurrentClient->getInputDevice()), this);
    dialog.exec();

//    if (dialog.exec() == QDialog::Accepted)  {
//        // Pass dialog values into class and try to open the port
//        m_pServer->addSerialClient(pSerialPort);

//        //AppSettings settings;
//        //settings.writeSerialSettings(QString::number(pSB->mountPt.getInstance())+"/COM", pSerialConfigDlg->settings().name);
//    }
}
