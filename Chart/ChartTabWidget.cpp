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
    addTab(pChart, pClient->getClientId());
}

void ChartTabWidget::removeClientChart(const QString &id)
{
    for(int i=0; i<this->count(); i++) {
        if(this->tabText(i) == id)
            this->removeTab(i);
    }
}

bool ChartTabWidget::hasClient(const QString &id)
{
    for(int i=0; i<this->count(); i++) {
        if(this->tabText(i) == id)
            return true;
    }

    return false;
}
