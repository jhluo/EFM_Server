#include "ClientCommandDialog.h"
#include "Client/AClient.h"
#include "Misc/AppSettings.h"
#include <QMessageBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>

ClientCommandDialog::ClientCommandDialog(AClient *pClient, QWidget *parent) :
    QDialog(parent),
    m_pClient(pClient)
{
    connect(this, SIGNAL(writeCommand(QString)), m_pClient, SLOT(sendData(QString)));
    connect(m_pClient, SIGNAL(bytesSent(int)), this, SLOT(onCommandSent(int)));
    connect(m_pClient, SIGNAL(clientAcknowledge(bool)), this, SLOT(onCommandAcknowledged(bool)));

    setWindowTitle(QString(tr("Send Client Command")));
    createActions();
}

void ClientCommandDialog::createActions()
{
    QLabel *pCommandLabel = new QLabel(QString(tr("Select command below:")), this);

    m_pCommandComboBox = new QComboBox(this);
    m_pCommandComboBox->addItem("dxsj02: 设置设备时间", eSetTime);
    m_pCommandComboBox->addItem("dxsj04: 传送采集数", eSendCount);
    m_pCommandComboBox->addItem("dxsj05: 清零", eReset);
    m_pCommandComboBox->addItem("dxsj06: 设置设备的ID号", eSetId);
    //m_pCommandComboBox->addItem("dxsj11: 设置电场仪底数", eSetLimit);
    //m_pCommandComboBox->addItem("dxsj16: 设置电场仪倍率", eSetMultiplier);

    m_pCommandComboBox->setCurrentIndex(-1);
    connect(m_pCommandComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCommandComboChanged()));

    m_pCommandEdit = new QLineEdit(this);

    m_pSendButton = new QPushButton(QString(tr("Send")), this);
    connect(m_pSendButton, SIGNAL(pressed()), this, SLOT(onSendButtonClicked()));

    m_pResultLabel = new QLabel;

    QVBoxLayout *pCommandLayout = new QVBoxLayout;
    pCommandLayout->addWidget(pCommandLabel);
    pCommandLayout->addWidget(m_pCommandComboBox);
    pCommandLayout->addWidget(m_pCommandEdit);
    pCommandLayout->addWidget(m_pSendButton);
    pCommandLayout->addWidget(m_pResultLabel);
    QGroupBox *pCommandGroupBox = new QGroupBox(QString(tr("Command")), this);
    pCommandGroupBox->setLayout(pCommandLayout);

    QGroupBox *pCommandDescriptionBox = new QGroupBox(QString(tr("Description")), this);
    m_pCommandDescription = new QLabel;
    m_pCommandDescription->setWordWrap(true);
    QHBoxLayout *pCommandDescriptionLayout = new QHBoxLayout;
    pCommandDescriptionLayout->addWidget(m_pCommandDescription);
    pCommandDescriptionBox->setLayout(pCommandDescriptionLayout);

    QHBoxLayout *pMainLayout = new QHBoxLayout;

    pMainLayout->addWidget(pCommandGroupBox);
    pMainLayout->addWidget(pCommandDescriptionBox);

    setLayout(pMainLayout);
}

void ClientCommandDialog::onSendButtonClicked()
{
    emit writeCommand(m_pCommandEdit->text());
}

void ClientCommandDialog::onCommandComboChanged()
{
    switch(m_pCommandComboBox->currentData().toInt()) {
        case eSetTime:
        {
            QDateTime currentDateTime = QDateTime::currentDateTime();
            QDate currentDate = currentDateTime.date();
            QTime currentTime = currentDateTime.time();
            QString command=QString("dxsj02:\"%1.%2.%3.%4.%5.%6.%7\"")
                    .arg(currentDate.year()-2000)
                    .arg(currentDate.month())
                    .arg(currentDate.day())
                    .arg(currentDate.dayOfWeek())
                    .arg(currentTime.hour())
                    .arg(currentTime.minute())
                    .arg(currentTime.second());
            m_pCommandEdit->setText(command);

            QString description = "设置设备的时间。\n";
            m_pCommandDescription->setText(description);
        break;
        }

        case eSendCount:
        {
            QString command=QString("dxsj04:\"1.0\"");
            m_pCommandEdit->setText(command);

            QString description = QString("1代表发送负离子采集数，0代表不发送正离子采集数\n"
                    "只有发送正负离子数时，发送的ID号是本机的ID号\n"
                    "发送负离子采集数时，机器的ID号是本机ID号加1\n"
                    "发送正离子采集数时，机器的ID号是本机ID号加2\n");
            m_pCommandDescription->setText(description);
        break;
        }

        case eReset:
        {
            QString command=QString("dxsj05:\"0\"");
            m_pCommandEdit->setText(command);

            QString description = QString("存储器清0, 清除存储器中的数据\n");
            m_pCommandDescription->setText(description);
        break;
        }

        case eSetId:
        {
            QString command=QString("dxsj06:\"05916\"");
            m_pCommandEdit->setText(command);

            QString description = QString("设置本机的ID号\n"
                                          "例如：dxsj06:\"05916\"，设置本机的ID号为05916"
                                          "ID号是5位数字，数字不足5位时，高位补零；最高位也可以是大小写英文字母。"
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eSetLimit:
        {
            QString command=QString("dxsj11:\"00000,00000\"");
            m_pCommandEdit->setText(command);

            QString description = QString("设置本机的ID号\n"
                                          "例如：dxsj06:\"05916\"，设置本机的ID号为05916"
                                          "ID号是5位数字，数字不足5位时，高位补零；最高位也可以是大小写英文字母。"
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eSetMultiplier:
        {
            QString command=QString("dxsj16:\"1.00,180,02.00;1.00,180,02.00\"");
            m_pCommandEdit->setText(command);

            QString description = QString("dxsj16:\"1.00,180,02.00;1.00,180,02.00\"\n"
                                          "第一组：1.00,180,02.00 负离子计算公式，1.00为负离子采集器阻抗，"
                                          "180为负离子采集器的风速1.8m/s，02.00为公式系数（浮点数，最大99.99）"
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        default:
        break;
    }
}

void ClientCommandDialog::onCommandSent(const int bytesWritten)
{
    if(bytesWritten == m_pCommandEdit->text().toLocal8Bit().size())
        m_pResultLabel->setText(QString(tr("Command was sent, awaiting acknowledgment from client.")));
    else
        m_pResultLabel->setText(QString(tr("Command was not sent correctly.  Please retry.")));
}

void ClientCommandDialog::onCommandAcknowledged(const bool ok)
{
    if(ok)
        m_pResultLabel->setText(QString(tr("Client acknowledge command.")));
    else
        m_pResultLabel->setText(QString(tr("Client could not execute command.  Please retry.")));
}
