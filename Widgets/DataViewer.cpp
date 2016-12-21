#include "DataViewer.h"
#include "Client/AClient.h"
#include "misc/Logger.h"

#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

DataViewer::DataViewer(AClient *pClient, QWidget *parent) :
    QDialog(parent),
    m_pClient(pClient),
    m_pMessageDisplay(NULL)
{
    createActions();
    populateDialog();

    //use this as the dataview for the client
    m_pClient->registerDataViewer(this->m_pMessageDisplay);
}

DataViewer::~DataViewer()
{
    //need to dereference this from the client or crash
    m_pClient->registerDataViewer(NULL);
}

void DataViewer::createActions()
{
    QLabel *pDescriptionLabel = new QLabel(QString(tr("Data input from client is displayed below:  ")));

    m_pMessageDisplay = new QTextEdit();
    m_pMessageDisplay->setReadOnly(true);

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    pMainLayout->addWidget(pDescriptionLabel);
    pMainLayout->addWidget(m_pMessageDisplay);

    this->setLayout(pMainLayout);
}

void DataViewer::populateDialog()
{
    //set the window title
    setWindowTitle(QString("设备 %1").arg(m_pClient->getClientId()));
    resize(640, 640);
}
