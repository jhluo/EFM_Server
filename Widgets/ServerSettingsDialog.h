#ifndef SERVERSETTINGSDIALOG_H
#define SERVERSETTINGSDIALOG_H

#include <QDialog>

//forward declaration
class QLineEdit;
class QDialogButtonBox;
class TheServer;

class ServerSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ServerSettingsDialog(TheServer *pServer, QWidget *pParent = 0);

private:
    QLineEdit* m_pHostEdit;
    QLineEdit* m_pPortEdit;
    QPushButton *m_pRestartButton;

    QDialogButtonBox* m_pButtonBox;

    TheServer *m_pServer;

    void createActions();
    void saveSettings();
    void loadSettings();

signals:

private slots:
    void onRestartServer();
    void accept();

};

#endif // SERVERSETTINGSDIALOG_H
