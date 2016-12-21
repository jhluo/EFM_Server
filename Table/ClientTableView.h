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
    AClientList *m_pClientList;

signals:
    void showChart(const bool enabled, AClient *pClient);

private slots:
    void showContextMenu(const QPoint&);    
    void onSendCommandTriggered();
    void onMessageViewerToggled(const bool enabled);
    void onShowChartToggled(const bool enabled);
    void onSerialConnectTriggered();
    void onSerialEditTriggered();
};
