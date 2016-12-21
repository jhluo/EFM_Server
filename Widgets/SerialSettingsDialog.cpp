#include "SerialSettingsDialog.h"
#include <QGroupBox>
#include <QFormLayout>

SerialSettingsDialog::SerialSettingsDialog(QSerialPort *pPort, QWidget *parent) :
    QDialog(parent),
    m_pSerialPort(pPort)
{
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
    pPortLayout->addRow(tr("COM Port:"), m_pPortCombo);
    pPortLayout->addRow(tr("Description:"), m_pPortDescriptionLabel);
    pPortLayout->addRow(tr("Manufacturer:"), m_pPortManufacturerLabel);
    pPortGroupBox->setLayout(pPortLayout);

    QGroupBox *pSettingsGroupBox = new QGroupBox(tr("Choose Port Settings"));
    m_pBaudRateCombo = new QComboBox;
    m_pBaudRateCombo->addItem("9600", QSerialPort::Baud9600);
    m_pBaudRateCombo->addItem("19200", QSerialPort::Baud19200);
    m_pBaudRateCombo->addItem("38400", QSerialPort::Baud38400);
    m_pBaudRateCombo->addItem("57600", QSerialPort::Baud57600);
    m_pBaudRateCombo->addItem("115200", QSerialPort::Baud115200);

    m_pDataBitsCombo = new QComboBox;
    m_pDataBitsCombo->addItem("5", QSerialPort::Data5);
    m_pDataBitsCombo->addItem("6", QSerialPort::Data6);
    m_pDataBitsCombo->addItem("7", QSerialPort::Data7);
    m_pDataBitsCombo->addItem("8", QSerialPort::Data8);

    m_pDataBitsCombo->setCurrentIndex(3); //default

    m_pParityCombo = new QComboBox;
    m_pParityCombo->addItem("None", QSerialPort::NoParity);
    m_pParityCombo->addItem("Even", QSerialPort::EvenParity);
    m_pParityCombo->addItem("Odd", QSerialPort::OddParity);
    m_pParityCombo->addItem("Mark", QSerialPort::MarkParity);
    m_pParityCombo->addItem("Space", QSerialPort::SpaceParity);

    m_pStopBitsCombo = new QComboBox;
    m_pStopBitsCombo->addItem("1", QSerialPort::OneStop);
    m_pStopBitsCombo->addItem("1.5", QSerialPort::OneAndHalfStop);
    m_pStopBitsCombo->addItem("2", QSerialPort::TwoStop);

    m_pFlowControlCombo = new QComboBox;
    m_pFlowControlCombo->addItem("None", QSerialPort::NoFlowControl);
    m_pFlowControlCombo->addItem("RTS/CTS", QSerialPort::HardwareControl);
    m_pFlowControlCombo->addItem("XON/XOFF", QSerialPort::SoftwareControl);

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

    //load the current port settings
    int index = -1;
    index = m_pPortCombo->findData(m_pSerialPort->portName());
    m_pPortCombo->setCurrentIndex(index);

    index = m_pBaudRateCombo->findData(m_pSerialPort->baudRate());
    m_pBaudRateCombo->setCurrentIndex(index);

    index = m_pDataBitsCombo->findData(m_pSerialPort->dataBits());
    m_pDataBitsCombo->setCurrentIndex(index);

    index = m_pParityCombo->findData(m_pSerialPort->parity());
    m_pParityCombo->setCurrentIndex(index);

    index = m_pStopBitsCombo->findData(m_pSerialPort->stopBits());
    m_pStopBitsCombo->setCurrentIndex(index);

    index = m_pFlowControlCombo->findData(m_pSerialPort->flowControl());
    m_pFlowControlCombo->setCurrentIndex(index);
}

void SerialSettingsDialog::apply()
{
    if (!m_pPortCombo->currentText().isEmpty()) {
        m_pSerialPort->setPortName(m_pPortCombo->currentText());

        m_pSerialPort->setBaudRate(static_cast<QSerialPort::BaudRate>(m_pBaudRateCombo->currentData().toInt()));

        m_pSerialPort->setDataBits(static_cast<QSerialPort::DataBits>(m_pDataBitsCombo->currentData().toInt()));

        m_pSerialPort->setParity(static_cast<QSerialPort::Parity>(m_pParityCombo->currentData().toInt()));

        m_pSerialPort->setStopBits(static_cast<QSerialPort::StopBits>(m_pBaudRateCombo->currentData().toInt()));

        m_pSerialPort->setFlowControl(static_cast<QSerialPort::FlowControl>(m_pBaudRateCombo->currentData().toInt()));

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
