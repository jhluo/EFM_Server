#ifndef CLIENTCHART_H
#define CLIENTCHART_H

#include <QObject>
#include <QChartView>
#include <QtCharts>
#include <QTimer>

class AClient;

class ClientChart : public QChartView
{
    Q_OBJECT

public:
    ClientChart(AClient *pClient, QWidget *pParent=0);
    ~ClientChart();

private:
    void createChart();
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    AClient *m_pClient;
    QChart *m_pChart;
    QLineSeries *m_pDataSeries;
    QTimer m_UpdateTimer;
    bool m_CenterChart;

    //these are used for mouse dragging
    bool m_isMousePressed;
    QPoint m_MousePressedPosition;

private slots:
    void updateChart(const QDateTime &time, const int value);
    void showContextMenu(const QPoint &pos);
    void onCenterChartToggled(const bool enabled);
};

#endif // CLIENTCHART_H
