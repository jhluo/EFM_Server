#include "ActivationDialog.h"
#include "Misc/AppSettings.h"
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMessageBox>

ActivationDialog::ActivationDialog(QWidget *parent) :
    QDialog(parent),
    m_pPasswordEdit(NULL),
    m_pConfirmButton(NULL)
{
    setWindowTitle(tr("Product Registration"));
    createActions();
}

void ActivationDialog::createActions()
{
    QLabel *pRegistrationLabel = new QLabel("Enter your registration code below:", this);

    m_pPasswordEdit = new QLineEdit(this);

    m_pConfirmButton = new QPushButton("Confirm", this);
    connect(m_pConfirmButton, SIGNAL(pressed()), this, SLOT(onConfirmButtonClicked()));

    QDialogButtonBox *pDialogButtons = new QDialogButtonBox(Qt::Horizontal);
    pDialogButtons->addButton(m_pConfirmButton, QDialogButtonBox::ActionRole);
    pDialogButtons->addButton(QDialogButtonBox::Cancel);
    connect(pDialogButtons, SIGNAL(rejected()), this, SLOT(reject()));

    QFormLayout *pMainLayout = new QFormLayout;

    pMainLayout->addRow(pRegistrationLabel);
    pMainLayout->addRow("Code:", m_pPasswordEdit);
    pMainLayout->addRow(pDialogButtons);

    setLayout(pMainLayout);
}

void ActivationDialog::onConfirmButtonClicked()
{
    AppSettings settings;
    settings.writeMiscSettings("productKey", m_pPasswordEdit->text());
    emit productKeyConfirmed(m_pPasswordEdit->text());

    accept();
}
