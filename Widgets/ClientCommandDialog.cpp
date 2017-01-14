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
    m_pCommandComboBox->addItem("SETCOM:  设置或读取设备的通讯参数", eSetCom);
    m_pCommandComboBox->addItem("AUTOCHCEK:  设备自检", eAutoCheck);
    m_pCommandComboBox->addItem("HELP:  帮助命令", eHelp);
    m_pCommandComboBox->addItem("QZ:  设置或读取设备的区站号", eQz);
    m_pCommandComboBox->addItem("ST:  设置或读取设备的服务类型", eSt);
    m_pCommandComboBox->addItem("DI:  读取设备标识位", eDi);
    m_pCommandComboBox->addItem("ID:  设置或读取设备ID", eId);
    m_pCommandComboBox->addItem("LAT:  设置或读取观测站的纬度", eLat);
    m_pCommandComboBox->addItem("LONG:  设置或读取观测站的经度", eLong);
    m_pCommandComboBox->addItem("DATE:  设置或读取设备日期", eDate);
    m_pCommandComboBox->addItem("TIME:  设置或读取设备时间", eTime);
    m_pCommandComboBox->addItem("DATETIME:  设置或读取设备日期与时间", eDateTime);
    m_pCommandComboBox->addItem("FTD:  设置或读取设备主动模式下的发送时间间隔", eFtd);
    m_pCommandComboBox->addItem("DOWN:  历史数据下载", eDown);
    m_pCommandComboBox->addItem("READDATA:  实时读取数据", eReadData);
    m_pCommandComboBox->addItem("SETCOMWAY:  设置握手机制方式", eSetComWay);


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
    emit writeCommand(m_pCommandEdit->text()+"\r\n");
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

        case eSetCom:
        {
            QString command=QString("SETCOM,9600,8,N,1");
            m_pCommandEdit->setText(command);
            QString description = QString("命令符：SETCOM\n"
                                          "参数：波特率 数据位 奇偶校验 停止位\n"
                                          "示例：若设备的波特率为9600 bps，数据位为8，奇偶校验为无，停止位为1，若对设备进行设置，键入命令为：\n"
                                          "SETCOM,9600,8,N,1\n"
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eAutoCheck:
        {
            QString command=QString("AUTOCHECK");
            m_pCommandEdit->setText(command);
            QString description = QString("返回的内容包括设备日期、时间，"
                                          "通讯端口的通讯参数，设备状态信息"
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eHelp:
        {
            QString command=QString("HELP");
            m_pCommandEdit->setText(command);
            QString description = QString("返回终端命令清单"
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eQz:
        {
            QString command=QString("QZ,57494");
            m_pCommandEdit->setText(command);
            QString description = QString("参数：设备区站号（5位字符）\n"
                                          "示例：若所属气象观测站的区站号为57494，则键入命令为：\n"
                                          "QZ,57494\n"
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eSt:
        {
            QString command=QString("ST,07");
            m_pCommandEdit->setText(command);
            QString description = QString("服务类型（2位数字）\n"
                                          "示例：若设备用于旅游气象站，则键入命令为：\n"
                                          "ST,07\n"
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eDi:
        {
            QString command=QString("DI");
            m_pCommandEdit->setText(command);
            QString description = QString("读取大气负离子自动观测仪设备标识位\n"
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eId:
        {
            QString command=QString("ID");
            m_pCommandEdit->setText(command);
            QString description = QString("参数：3位数字\n"
                                          "示例：大气负离子自动观测仪设备ID为：000，对设备进行设置，键入命令为：\n"
                                          "ID,000\n"
                                          "若为读取设备ID参数，直接键入命令：\n"
                                          "ID\n"
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eLat:
        {
            QString command=QString("LAT");
            m_pCommandEdit->setText(command);
            QString description = QString("参数：DD.MM.SS（DD为度，MM为分，SS为秒）\n"
                                          "示例：若所属观测站的纬度为32°14′20″，则键入命令为：LAT,32.14.20\n"
                                          "若设备数据采集器中的纬度为42°06′00″，直接键入命令：\n"
                                          "LAT\n"
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eLong:
        {
            QString command=QString("LONG");
            m_pCommandEdit->setText(command);
            QString description = QString("参数：DD.MM.SS（DD为度，MM为分，SS为秒）\n"
                                          "示例：若所属观测站的经度为116°34′18″，则键入命令为：LONG,116.34.18\n"
                                          "若设备数据采集器中的经度为108°32′03″，直接键入命令：\n"
                                          "LONG\n"
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eDate:
        {
            QDateTime currentDateTime = QDateTime::currentDateTime();
            QDate currentDate = currentDateTime.date();
            QString dateStr = currentDate.toString("yyyy-MM-dd");
            QString command=QString("DATE,%1")
                    .arg(dateStr);
            m_pCommandEdit->setText(command);
            QString description = QString("参数：YYYY-MM-DD（YYYY为年，MM为月，DD为日）\n"
                                          "示例：若对设备设置的日期为2012年7月21日，键入命令为：\n"
                                          "DATE,2012-07-21\n"
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eTime:
        {
            QDateTime currentDateTime = QDateTime::currentDateTime();
            QTime currentTime = currentDateTime.time();
            QString timeStr = currentTime.toString("HH:mm:ss");
            QString command=QString("TIME,%1")
                    .arg(timeStr);
            m_pCommandEdit->setText(command);
            QString description = QString("参数：HH:MM:SS（HH为时，MM为分，SS为秒）\n"
                                          "示例：若对设备设置的时间为12时34分00秒，键入命令为：\n"
                                          "TIME,12:34:00\n"
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eDateTime:
        {
            QDateTime currentDateTime = QDateTime::currentDateTime();
            QDate currentDate = currentDateTime.date();
            QString dateStr = currentDate.toString("yyyy-MM-dd");
            QTime currentTime = currentDateTime.time();
            QString timeStr = currentTime.toString("HH:mm:ss");
            QString command=QString("TIME,%1,%2")
                    .arg(timeStr)
                    .arg(dateStr);
            m_pCommandEdit->setText(command);
            QString description = QString("参数：YYYY-MM-DD,HH:MM:SS（YYYY为年，MM为月，DD为日, HH为时，MM为分，SS为秒）\n"
                                          "示例：若对设备设置的日期为2013年5月27日12时34分00秒，键入命令为：\n"
                                          "DATETIME,2013-05-27,12:34:00\n"
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eFtd:
        {
            QString command=QString("FTD");
            m_pCommandEdit->setText(command);
            QString description = QString(""
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eDown:
        {
            QString command=QString("DOWN");
            m_pCommandEdit->setText(command);
            QString description = QString(""
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eReadData:
        {
            QString command=QString("READDATA");
            m_pCommandEdit->setText(command);
            QString description = QString(""
                                          );
            m_pCommandDescription->setText(description);
        break;
        }

        case eSetComWay:
        {
            QString command=QString("SETCOMWAY,1");
            m_pCommandEdit->setText(command);
            QString description = QString("参数为：1为主动发送方式，0为被动读取方式，\n"
                                          "示例：设备默认为被动读取方式，"
                                          "如果需要采用主动发送方式可以由上位机\n"
                                          "发送命令“SETCOMWAY,1”, "
                                          "则上位机实时接收设备主动发送的数据即可，如需要采用被动读取方式，"
                                          "则上位机发送命令“SETCOMWAY,0”."
                                          "第一次连接设备时默认为被动读取方式，上位机不用发送“SETCOMWAY,0”命令。\n"
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
