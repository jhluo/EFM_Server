#include "MainWindowWidget.h"
#include "TheServer.h"
#include "ClientTableWidget.h"

#include "Misc/Logger.h"
#include "Misc/AppSettings.h"
#include "Chart/ChartDialog.h"

#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QDesktopServices>
#include <QDir>
#include <QMessageBox>

MainWindowWidget::MainWindowWidget(TheServer *pApp, QWidget *parent)
    : QWidget(parent),
      m_pServer(pApp),
      m_pChartDialog(NULL)
{
    setMinimumSize(600, 420);
    createLayout();
    populateWidgets();

    //create chart dialog in the background
    m_pChartDialog = new ChartDialog(this);
    //connect signal to add tabs to the widget
    connect(m_pClientTable, SIGNAL(showChart(bool,AClient*)), m_pChartDialog, SLOT(onChartToggled(bool,AClient*)));
}

MainWindowWidget::~MainWindowWidget()
{

}

void MainWindowWidget::createLayout()
{
    m_pClientTable = new ClientTableWidget(m_pServer, this);
    //m_pClientTableView = new ClientTableView(m_pServer->getClientList(), this);

    m_pDatabaseCheckBox = new QCheckBox(tr("Write to Database"));
    connect(m_pDatabaseCheckBox, SIGNAL(toggled(bool)), this, SLOT(onDatabaseChecked(bool)));

    m_pLogCheckBox = new QCheckBox(tr("Log client data"));
    connect(m_pLogCheckBox, SIGNAL(toggled(bool)), this, SLOT(onLoggingChecked(bool)));

    m_pRawCheckBox = new QCheckBox(tr("Log raw client data"));
    connect(m_pRawCheckBox, SIGNAL(toggled(bool)), this, SLOT(onRawLoggingChecked(bool)));

    m_pLogButton = new QPushButton(tr("Show Log Files"));
    connect(m_pLogButton, SIGNAL(pressed()), this, SLOT(onLogButtonPushed()));

    m_pChartButton = new QPushButton(tr("Show Chart"));
    connect(m_pChartButton, SIGNAL(pressed()), this, SLOT(onChartButtonPushed()));

    m_pWebButton = new QPushButton(tr("Web Interface"));
    connect(m_pWebButton, SIGNAL(pressed()), this, SLOT(onWebButtonPushed()));

    QVBoxLayout *pCommandLayout = new QVBoxLayout;
    pCommandLayout->addWidget(m_pDatabaseCheckBox);
    pCommandLayout->addWidget(m_pRawCheckBox);
    pCommandLayout->addWidget(m_pLogCheckBox);
    pCommandLayout->addWidget(m_pLogButton);
    pCommandLayout->addWidget(m_pChartButton);
    pCommandLayout->addWidget(m_pWebButton);

    QGroupBox *pCommandGroupBox = new QGroupBox(tr("Command"));
    pCommandGroupBox->setLayout(pCommandLayout);

    QHBoxLayout *pTopLayout = new QHBoxLayout;
    pTopLayout->addWidget(m_pClientTable);
    pTopLayout->addWidget(pCommandGroupBox);
    QWidget *pTopWidget = new QWidget;
    pTopWidget->setLayout(pTopLayout);

    m_pLogEdit = new QTextEdit(this);
    m_pLogEdit->setReadOnly(true);
    Logger::getInstance()->registerSystemDisplay(m_pLogEdit);

    //show time stamp on log
    Logger::getInstance()->setShowTime(true);

    //user a splitter
    QSplitter *pSplitter = new QSplitter(Qt::Vertical, this);
    pSplitter->addWidget(pTopWidget);
    pSplitter->addWidget(m_pLogEdit);
    pSplitter->setStretchFactor(0, 1);
    pSplitter->setCollapsible(0, false);

    QVBoxLayout *pMainLayout = new QVBoxLayout();
    pMainLayout->addWidget(pSplitter);

    setLayout(pMainLayout);
}

void MainWindowWidget::populateWidgets()
{
    AppSettings settings;

    bool writeDatabase = settings.readMiscSettings("writeDatabase", true).toBool();
    m_pDatabaseCheckBox->setChecked(writeDatabase);

    bool dataLog = settings.readMiscSettings("dataLog", false).toBool();
    m_pLogCheckBox->setChecked(dataLog);

    bool rawLog = settings.readMiscSettings("rawLog", false).toBool();
    m_pRawCheckBox->setChecked(rawLog);
}

void MainWindowWidget::onDatabaseChecked(const bool checked)
{
    AppSettings settings;
    settings.writeMiscSettings("writeDatabase", checked);
}

void MainWindowWidget::onLoggingChecked(const bool checked)
{
    AppSettings settings;
    settings.writeMiscSettings("dataLog", checked);
}

void MainWindowWidget::onRawLoggingChecked(const bool checked)
{
    AppSettings settings;
    settings.writeMiscSettings("rawLog", checked);
}

void MainWindowWidget::onLogButtonPushed()
{
    if(QDir("log").exists()) {
      QDesktopServices::openUrl(QUrl("log"));
    } else {
      QMessageBox::critical(0,tr("No log files"), tr("No log file exists."));
    }
}

void MainWindowWidget::onChartButtonPushed()
{
    if (m_pChartDialog!=NULL) {
        m_pChartDialog->show();
    }
}

void MainWindowWidget::onWebButtonPushed()
{
    QUrl link(QString("http://bjepex.oicp.net:60086/flztest4/4SManager.aspx"));
    QDesktopServices::openUrl(link);
}
