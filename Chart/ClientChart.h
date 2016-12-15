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

    AClient *m_pClient;
    QChart *m_pChart;
    QLineSeries *m_pDataSeries;
    QTimer m_UpdateTimer;

private slots:
    void updateChart(const QDateTime &time, const int value);
};

#endif // CLIENTCHART_H
