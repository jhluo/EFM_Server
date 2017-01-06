#include <QFormLayout>

#include "OffsetSettingsDialog.h"
#include "Misc/OffsetSettings.h"
#include "Client/AClient.h"

OffsetSettingsDialog::OffsetSettingsDialog(AClient *pClient, QWidget *pParent) :
    QDialog(pParent),
    m_pClient(pClient)
{
    setWindowTitle(tr("Offset Settings"));
    createActions();
}

void OffsetSettingsDialog::createActions()
{
    m_pBaseEdit = new QLineEdit(this);
    m_pMultiplierEdit = new QLineEdit(this);

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel,Qt::Horizontal);
    connect(m_pButtonBox, SIGNAL(accepted()),this, SLOT(accept()));
    connect(m_pButtonBox, SIGNAL(rejected()),this, SLOT(reject()));

    QFormLayout *pLayout = new QFormLayout(this);
    pLayout->addRow(tr("Base"), m_pBaseEdit);
    pLayout->addRow(tr("Multiplier"), m_pMultiplierEdit);
    pLayout->addRow(m_pButtonBox);

    setLayout(pLayout);

    loadSettings();
}


void OffsetSettingsDialog::saveSettings()
{
    OffsetSettings settings;
    settings.writeBaseOffset(m_pClient->getClientId(), m_pBaseEdit->text().toDouble());
    settings.writeMultiplierOffset(m_pClient->getClientId(), m_pMultiplierEdit->text().toDouble());
}

void OffsetSettingsDialog::loadSettings()
{
    OffsetSettings settings;
    double base = settings.readBaseOffset(m_pClient->getClientId());
    double multiplier = settings.readMultiplierOffset(m_pClient->getClientId());

    m_pBaseEdit->setText(QString::number(base));
    m_pMultiplierEdit->setText(QString::number(multiplier));
}


void OffsetSettingsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}
