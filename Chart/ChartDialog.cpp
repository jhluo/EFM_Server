#include "ChartDialog.h"
#include "ChartTabWidget.h"
#include "AClient.h"
#include <QHBoxLayout>

ChartDialog::ChartDialog(QWidget *pParent)
    :QDialog(pParent),
    m_pTabWidget(NULL)
{
    createActions();
    setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
    setMinimumSize(640, 480);
    resize(800, 600);
}

ChartDialog::~ChartDialog()
{

}

void ChartDialog::createActions()
{
    m_pTabWidget = new ChartTabWidget(this);

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    pMainLayout->addWidget(m_pTabWidget);

    setLayout(pMainLayout);
}

void ChartDialog::onChartToggled(const bool enabled, AClient *pClient)
{
    if(enabled) {
        if(!m_pTabWidget->hasClient(pClient->getClientId())) {
            m_pTabWidget->addClientChart(pClient);
        }
    } else {
        m_pTabWidget->removeClientChart(pClient->getClientId());
        if(m_pTabWidget->count()==0) {
            this->close();
        }
    }
}
