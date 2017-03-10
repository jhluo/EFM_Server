#include "MainStatusbar.h"

MainStatusbar::MainStatusbar(QWidget *parent):
    QStatusBar(parent)
{
    createLayout();
    startTimer(1000);
}

MainStatusbar::~MainStatusbar()
{

}

void MainStatusbar::createLayout()
{
    m_pCompanyLabel = new QLabel("北京依派伟业数码科技有限公司 010-82564248", this);
    m_pTimeLabel = new QLabel(this);

    this->addWidget(m_pTimeLabel);
    this->addPermanentWidget(m_pCompanyLabel);
}

void MainStatusbar::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    updateLabels();
}

void MainStatusbar::updateLabels()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    m_pTimeLabel->setText(currentTime.toString("yyyy-MM-dd HH:mm:ss"));
}
