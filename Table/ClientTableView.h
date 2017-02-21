#pragma once

#include <QTableView>
#include "ClientTableModel.h"

class AClient;
class TheServer;

class ClientTableView : public QTableView
{
    Q_OBJECT

public:
    ClientTableView(TheServer *pServer, QWidget *pParent = 0);
    ~ClientTableView();

private:
    void setupTable();
    void mousePressEvent(QMouseEvent *event);

    ClientTableModel *m_pClientTableModel;
    TheServer *m_pServer;

signals:
    void showChart(const bool enabled, AClient *pClient);

private slots:
    void showContextMenu(const QPoint&);    
    void onSendCommandTriggered();
    void onConfigClientTriggered();
    void onMessageViewerToggled(const bool enabled);
    void onShowChartToggled(const bool enabled);
    void onSerialConnectTriggered();
    void onSerialEditTriggered();
};
