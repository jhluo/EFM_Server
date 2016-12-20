#ifndef CLIENTTABLEWIDGET_H
#define CLIENTTABLEWIDGET_H

#include <QWidget>
#include <QTableWidget>

class TheServer;
class AClient;

class ClientTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    ClientTableWidget(TheServer *pServer, QWidget *pParent = 0);
    ~ClientTableWidget();

private:
    void setupTable();

    void mousePressEvent(QMouseEvent *event);

    //This is the data source
    TheServer *m_pServer;

    //How often we update the table
    QTimer *m_pUpdateTimer;

signals:
    void showChart(const bool enabled, AClient *pClient);

private slots:
    void showContextMenu(const QPoint&);
    void updateTable();

    void onMessageViewerTriggered();
    void onSendCommandTriggered();
    void onShowChartToggled(const bool enabled);
};


#endif // CLIENTTABLEWIDGET_H
