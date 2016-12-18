#include "SerialSettingsDialog.h"
#include <QGroupBox>
#include <QFormLayout>

SerialSettingsDialog::SerialSettingsDialog(QWidget *parent) :
    QDialog(parent)
{
    m_pSerialPort = new QSerialPort;

    createActions();
    populateDialog();
}

SerialSettingsDialog::~SerialSettingsDialog()
{
}

void SerialSettingsDialog::createActions()
{
    QGroupBox *pPortGroupBox = new QGroupBox(tr("Select Serial Port"));
    m_pPortCombo = new QComboBox;
    m_pPortCombo->setInsertPolicy(QComboBox::InsertAlphabetically);
    connect(m_pPortCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onPortComboBoxIndexChanged(int)));

    m_pPortDescriptionLabel = new QLabel;
    m_pPortManufacturerLabel = new QLabel;
    QFormLayout *pPortLayout = new QFormLayout;
    pPortLayout->addRow("COM Port:", m_pPortCombo);
    pPortLayout->addRow("Description:", m_pPortDescriptionLabel);
    pPortLayout->addRow("Manufacturer:", m_pPortManufacturerLabel);
    pPortGroupBox->setLayout(pPortLayout);

    QGroupBox *pSettingsGroupBox = new QGroupBox(tr("Choose Port Settings"));
    m_pBaudRateCombo = new QComboBox;
    QStringList baudRateList;
    baudRateList << "9600" << "19200" << "38400" << "57600" << "76800" << "115200";
    m_pBaudRateCombo->addItems(baudRateList);

    m_pDataBitsCombo = new QComboBox;
    QStringList databitList;
    databitList << "5" << "6" << "7" << "8";
    m_pDataBitsCombo->addItems(databitList);
    m_pDataBitsCombo->setCurrentIndex(3); //default

    m_pParityCombo = new QComboBox;
    QStringList parityList;
    parityList << "None" << "Even" << "Odd" << "Mark" << "Space";
    m_pParityCombo->addItems(parityList);

    m_pStopBitsCombo = new QComboBox;
    QStringList stopbitList;
    stopbitList << "1" << "1.5" << "2";
    m_pStopBitsCombo->addItems(stopbitList);

    m_pFlowControlCombo = new QComboBox;
    QStringList flowList;
    flowList << "None" << "RTS/CTS" << "XON/XOFF";
    m_pFlowControlCombo->addItems(flowList);

    m_pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    m_pButtonBox->addButton(QDialogButtonBox::Ok);
    m_pButtonBox->addButton(QDialogButtonBox::Cancel);


    connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(apply()));
    connect(m_pButtonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QFormLayout *pSettingsLayout = new QFormLayout;
    pSettingsLayout->addRow(tr("Baud Rate"), m_pBaudRateCombo);
    pSettingsLayout->addRow(tr("Data Bits"), m_pDataBitsCombo);
    pSettingsLayout->addRow(tr("Parity"), m_pParityCombo);
    pSettingsLayout->addRow(tr("Stop Bits"), m_pStopBitsCombo);
    pSettingsLayout->addRow(tr("Flow Control"), m_pFlowControlCombo);
    pSettingsLayout->addRow(m_pButtonBox);

    pSettingsGroupBox->setLayout(pSettingsLayout);

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    pMainLayout->addWidget(pPortGroupBox);
    pMainLayout->addWidget(pSettingsGroupBox);

    this->setLayout(pMainLayout);
}

void SerialSettingsDialog::populateDialog()
{
    m_pPortCombo->clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        m_pPortCombo->addItem(info.portName());
        if (info.isBusy()) {
            //this is to grey out ports that are being used by other programs
            m_pPortCombo->setItemData(m_pPortCombo->count()-1, 0, Qt::UserRole - 1);
        }
    }

    // Handle case when there are no port for use
    if (m_pPortCombo->count() == 0)
        m_pPortDescriptionLabel->setText(tr("Description: <b>NO PORTS WERE FOUND</b>"));

    m_pPortCombo->setCurrentIndex(-1);
}

void SerialSettingsDialog::apply()
{
    if (!m_pPortCombo->currentText().isEmpty()) {
        m_pSerialPort->setPortName(m_pPortCombo->currentText());
        m_pSerialPort->setBaudRate(m_pBaudRateCombo->currentText().toInt());

        m_pSerialPort->setDataBits(static_cast<QSerialPort::DataBits>(m_pBaudRateCombo->currentText().toInt()));

        if(m_pParityCombo->currentIndex() == 0)
            m_pSerialPort->setParity(QSerialPort::NoParity);
        else
            m_pSerialPort->setParity(static_cast<QSerialPort::Parity>(m_pBaudRateCombo->currentIndex()+1));

        if(m_pStopBitsCombo->currentText() == "1.5")
            m_pSerialPort->setStopBits(QSerialPort::OneAndHalfStop);
        else
            m_pSerialPort->setStopBits(static_cast<QSerialPort::StopBits>(m_pBaudRateCombo->currentText().toInt()));

        m_pSerialPort->setFlowControl(static_cast<QSerialPort::FlowControl>(m_pBaudRateCombo->currentIndex()));

        emit newSerialPort(m_pSerialPort);
        accept();
    } else {
        reject();
    }
}

void SerialSettingsDialog::onPortComboBoxIndexChanged(int index)
{
    if (index == -1) {
        m_pPortDescriptionLabel->clear();
        m_pPortManufacturerLabel->clear();
    }

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if (info.portName() == m_pPortCombo->currentText()) {
            m_pPortDescriptionLabel->setText(info.description());
            m_pPortManufacturerLabel->setText(info.manufacturer());
        }
    }
}
