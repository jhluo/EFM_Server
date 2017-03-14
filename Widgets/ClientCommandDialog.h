#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

class AClient;

class ClientCommandDialog : public QDialog
{
    Q_OBJECT

public:
    enum eCommands{
        eSetTime,
        eSendCount,
        eReset,
        eSetId,
        eSetLimit,
        eSetMultiplier,
        eSetCom,
        eAutoCheck,
        eHelp,
        eQz,
        eSt,
        eDi,
        eId,
        eLat,
        eLong,
        eDate,
        eTime,
        eDateTime,
        eFtd,
        eDown,
        eReadData,
        eSetComWay,

        eTotalNumber
    };

    explicit ClientCommandDialog(AClient *pClient, QWidget *parent = 0);

private:
    void createActions();

    QComboBox *m_pCommandComboBox;
    QLineEdit *m_pCommandEdit;
    QLabel *m_pCommandDescription;
    QPushButton *m_pSendButton;
    QLabel *m_pResultLabel;

    AClient *m_pClient;

signals:
    writeCommand(const QString &data, const QString &ack);

public slots:

private slots:
    void onCommandComboChanged();
    void onSendButtonClicked();

    void onCommandSent(const int bytesWritten);
    void onCommandAcknowledged(const bool ok);

    void onClientDisconnected();
};
