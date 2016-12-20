#pragma once

#include <QTableView>
#include "ClientTableModel.h"

class AClient;
class AClientList;

class ClientTableView : public QTableView
{
    Q_OBJECT

public:
    ClientTableView(AClientList *pClientList, QWidget *pParent = 0);
    ~ClientTableView();

private:
    void setupTable();
    void mousePressEvent(QMouseEvent *event);

    ClientTableModel *m_pClientTableModel;

private slots:
    void showContextMenu(const QPoint&);
    void onMessageViewerTriggered();
};
