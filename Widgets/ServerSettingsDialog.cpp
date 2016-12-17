#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QtSql>
#include <QDir>
#include <QMessageBox>

#include "ServerSettingsDialog.h"
#include "Misc/AppSettings.h"
#include "Misc/Logger.h"
#include "TheServer.h"

ServerSettingsDialog::ServerSettingsDialog(TheServer *pServer, QWidget *pParent) :
    QDialog(pParent),
    m_pServer(pServer)
{
    setWindowTitle(tr("Server Settings"));
    createActions();
}

void ServerSettingsDialog::createActions()
{
    AppSettings settings;

    m_pHostEdit = new QLineEdit(this);
    m_pPortEdit = new QLineEdit(this);

    m_pRestartButton = new QPushButton(tr("Restart Server"), this);
    connect(m_pRestartButton, SIGNAL(pressed()), this, SLOT(onRestartServer()));

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel,Qt::Horizontal);
    connect(m_pButtonBox, SIGNAL(accepted()),this, SLOT(accept()));
    connect(m_pButtonBox, SIGNAL(rejected()),this, SLOT(reject()));

    QFormLayout *pLayout = new QFormLayout(this);
    pLayout->addRow(tr("Host"), m_pHostEdit);
    pLayout->addRow(tr("Port"), m_pPortEdit);
    pLayout->addRow(m_pRestartButton);
    pLayout->addRow(m_pButtonBox);

    setLayout(pLayout);

    loadSettings();
}


void ServerSettingsDialog::saveSettings()
{
    AppSettings appSet;

    appSet.writeServerSettings("host", m_pHostEdit->text());
    appSet.writeServerSettings("port", m_pPortEdit->text());
}

void ServerSettingsDialog::loadSettings()
{
    AppSettings appSet;

    m_pHostEdit->setText(appSet.readServerSettings("host", "127.0.0.1").toString());
    m_pPortEdit->setText(appSet.readServerSettings("port", "5101").toString());
}

void ServerSettingsDialog::onRestartServer()
{    
    saveSettings();
    m_pServer->shutdownServer();
    m_pServer->startServer();
}

void ServerSettingsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}
