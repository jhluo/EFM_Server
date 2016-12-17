#pragma once

#include <QTabWidget>
#include <QVector>
#include <QChartView>
#include <QtCharts>

class ClientChart;
class AClient;

class ChartTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    // Constructor contains normal init() items as well
    explicit ChartTabWidget(QWidget *pParent=0);
    ~ChartTabWidget();

    void addClientChart(AClient *pClient);
    void removeClientChart(const int id);

    bool hasClient(const int id);

private:


signals:


};

