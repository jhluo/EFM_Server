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
    m_pChart->addSeries(m_pDataSeries);

    m_pChart->createDefaultAxes();

    QDateTimeAxis *pTimeAxis = new QDateTimeAxis;
    //pTimeAxis->setFormat("hh:mm");
    pTimeAxis->setFormat("dd-MM-yyyy h:mm");
    m_pChart->setAxisX(pTimeAxis, m_pDataSeries);
    m_pDataSeries->attachAxis(pTimeAxis);
    m_pChart->axisY()->setRange(0, 1000);;

    this->setChart(m_pChart);
}

void ClientChart::updateChart(const QDateTime &time, const int value)
{
    int seconds = m_pClient->getClientConnectTime().secsTo(time);
    m_pDataSeries->append(seconds, value);
    m_pDataSeries->append(time.toMSecsSinceEpoch(), value);
}
