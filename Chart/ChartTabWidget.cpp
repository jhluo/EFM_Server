#include "ChartTabWidget.h"
#include "ClientChart.h"
#include "AClient.h"


ChartTabWidget::ChartTabWidget(QWidget *pParent)
    :QTabWidget(pParent)
{

}

ChartTabWidget::~ChartTabWidget()
{

}

void ChartTabWidget::addClientChart(AClient *pClient)
{
    ClientChart *pChart = new ClientChart(pClient, this);
    addTab(pChart, QString::number(pClient->getClientId()));
}

void ChartTabWidget::removeClientChart(const int id)
{
    for(int i=0; i<this->count(); i++) {
        if(this->tabText(i) == QString::number(id))
            this->removeTab(i);
    }
}

bool ChartTabWidget::hasClient(const int id)
{
    for(int i=0; i<this->count(); i++) {
        if(this->tabText(i) == QString::number(id))
            return true;
    }

    return false;
}
