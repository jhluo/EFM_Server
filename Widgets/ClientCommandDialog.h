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
        eSetMultiplier
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
    writeCommand(const QString &data);

public slots:

private slots:
    void onCommandComboChanged();
    void onSendButtonClicked();

    void onCommandSent(const int bytesWritten);
    void onCommandAcknowledged(const bool ok);
};
