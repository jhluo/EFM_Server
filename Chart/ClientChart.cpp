#include "ClientChart.h"
#include "AClient.h"

ClientChart::ClientChart(AClient *pClient, QWidget *pParent)
    :QChartView(pParent),
      m_pClient(pClient),
      m_pChart(NULL)
{
    createChart();
    connect(m_pClient, SIGNAL(receivedData(QDateTime,int)), this, SLOT(updateChart(QDateTime,int)));
}

ClientChart::~ClientChart()
{
    delete m_pChart;
}

void ClientChart::createChart()
{
    m_pDataSeries = new QLineSeries();
    QPen red(Qt::red);
    red.setWidth(2);
    m_pDataSeries->setPen(red);

    m_pChart= new QChart();
    m_pChart->setTitle(QString("Client %1 Negative Ion").arg(QString::number(m_pClient->getClientId())));
    m_pChart->setAnimationOptions(QChart::GridAxisAnimations);
    m_pChart->legend()->hide();
    m_pChart->addSeries(m_pDataSeries);

    QDateTimeAxis *pTimeAxisX = new QDateTimeAxis;
    pTimeAxisX->setTickCount(10);
    pTimeAxisX->setFormat("MM-dd hh:mm");
    pTimeAxisX->setTitleText("Time");
    pTimeAxisX->setRange(QDateTime::currentDateTime(), QDateTime::currentDateTime().addSecs(60*5));
    m_pChart->addAxis(pTimeAxisX, Qt::AlignBottom);
    m_pDataSeries->attachAxis(pTimeAxisX);

    QValueAxis *pCountAxisY = new QValueAxis;
    pCountAxisY->setLabelFormat("%i");
    pCountAxisY->setTitleText("N Ion count");
    pCountAxisY->setRange(0, 1000);
    m_pChart->addAxis(pCountAxisY, Qt::AlignLeft);
    m_pDataSeries->attachAxis(pCountAxisY);

    this->setChart(m_pChart);
    this->setRenderHint(QPainter::Antialiasing);
}

void ClientChart::updateChart(const QDateTime &time, const int value)
{
    Q_UNUSED(time);

    m_pDataSeries->append(QDateTime::currentMSecsSinceEpoch(), value);
}
