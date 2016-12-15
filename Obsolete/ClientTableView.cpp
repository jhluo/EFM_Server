#include "ClientTableView.h"
#include "Misc/Logger.h"
#include "Widgets/DataViewer.h"
#include "AClient.h"

#include <QHeaderView>
#include <QAction>
#include <QInputEvent>
#include <QMenu>

ClientTableView::ClientTableView(QList<AClient*> *pClientList, QWidget *pParent)
    : QTableView(pParent)
{
    m_pClientTableModel = new ClientTableModel(this);
    m_pClientTableModel->setClientList(pClientList);
    setModel(m_pClientTableModel);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(showContextMenu(QPoint)));

    setupTable();
}

ClientTableView::~ClientTableView()
{

}

void ClientTableView::setupTable()
{
    //this->verticalHeader()->setVisible(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
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

    QMenu *pMenu = new QMenu(this);
    QAction *pMsgViewAction = new QAction(QString("View Data..."), pMenu);
    connect(pMsgViewAction, SIGNAL(triggered()), this, SLOT(onMessageViewerTriggered()));
    pMenu->addAction(pMsgViewAction);
    pMenu->exec(QCursor::pos());
}

//Open a message viewer dialg when option selected
void ClientTableView::onMessageViewerTriggered()
{
    //use this dialog to show client data

    //modaless
    //DataViewer viewer(m_pServer->getClient(currentRow()), this);
    //viewer.exec();

    //modal
    //DataViewer *pViewer = new DataViewer(m_pServer->getClient(currentRow()), this);
    //pViewer->setAttribute( Qt::WA_DeleteOnClose, true);
    //pViewer->show();
}
