#include "ClientChart.h"
#include "AClient.h"

#define MAX_CHART_POINTS 3600 * 1 //1 hour

ClientChart::ClientChart(AClient *pClient, QWidget *pParent)
    :QChartView(pParent),
      m_pClient(pClient),
      m_pChart(NULL),
      m_CenterChart(true),
      m_isMousePressed(false)
{
    createChart();
    connect(m_pClient, SIGNAL(receivedData(QDateTime,int)), this, SLOT(updateChart(QDateTime,int)));

    //right click menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(showContextMenu(QPoint)));
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
    m_pChart->setTitle(QString("Client %1 Negative Ion").arg(m_pClient->getClientId()));
    m_pChart->setAnimationOptions(QChart::NoAnimation);
    m_pChart->legend()->hide();
    m_pChart->addSeries(m_pDataSeries);

    QDateTimeAxis *pTimeAxisX = new QDateTimeAxis;
    pTimeAxisX->setTickCount(10);
    pTimeAxisX->setFormat("hh:mm:ss");
    pTimeAxisX->setTitleText("Time");
    pTimeAxisX->setRange(QDateTime::currentDateTime().addSecs(-60*3), QDateTime::currentDateTime().addSecs(60*3));
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
    this->setRubberBand(QChartView::RectangleRubberBand);
}

void ClientChart::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        m_isMousePressed = true;
        m_MousePressedPosition = event->globalPos();
    }

    QWidget::mousePressEvent(event);
}

void ClientChart::mouseReleaseEvent(QMouseEvent *event)
{
    m_isMousePressed = false;
    QWidget::mousePressEvent(event);
}

void ClientChart::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isMousePressed && !m_CenterChart) {  //can't move the chart if it's center
        QPoint currPos = event->globalPos();
        if(currPos != m_MousePressedPosition) {
            QPoint diff = m_MousePressedPosition - currPos;
            m_pChart->scroll(diff.x(), -diff.y());
            m_MousePressedPosition = currPos;
        }
    }

    QWidget::mouseMoveEvent(event);
}

void ClientChart::wheelEvent(QWheelEvent *event)
{
    if(event->delta()>0) {
        m_pChart->zoomIn();
    } else {
        m_pChart->zoomOut();
    }
}

void ClientChart::showContextMenu(const QPoint &pos)
{
    Q_UNUSED(pos);
    //make sure there is someting to be selected
    QMenu *pMenu = new QMenu(this);

    //add action to toggle always centering chart
    QAction *pCenterChartAction = new QAction(QString(tr("Center Chart")), pMenu);
    pCenterChartAction->setCheckable(true);
    pCenterChartAction->setChecked(m_CenterChart);
    connect(pCenterChartAction, SIGNAL(toggled(bool)), this, SLOT(onCenterChartToggled(bool)));
    pMenu->addAction(pCenterChartAction);

    pMenu->exec(QCursor::pos());
}

void ClientChart::updateChart(const QDateTime &time, const int value)
{
    Q_UNUSED(time);

    m_pDataSeries->append(QDateTime::currentMSecsSinceEpoch(), value);

    //limit total number of points in chart
    if(m_pDataSeries->pointsVector().size() >= MAX_CHART_POINTS) {
        m_pDataSeries->remove(0);
    }

    if(m_CenterChart) {
        QPointF oldCenter = QPointF(m_pChart->plotArea().right(), m_pChart->plotArea().center().y());
        QPointF newCenter = m_pChart->mapToPosition(QPointF(QDateTime::currentMSecsSinceEpoch(), value), m_pDataSeries);
        QPointF diff = newCenter-oldCenter;
        m_pChart->scroll(diff.x(), -diff.y());
    }
}

void ClientChart::onCenterChartToggled(const bool enabled)
{
    m_CenterChart = enabled;
}
