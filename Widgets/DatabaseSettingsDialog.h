#ifndef DATABASESETTINGSDIALOG_H
#define DATABASESETTINGSDIALOG_H

#include <QDialog>

//forward declaration
class QLineEdit;
class QDialogButtonBox;

class DatabaseSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseSettingsDialog(QWidget *pParent = 0);

private:
    QLineEdit* m_pDBNameEdit;
    QLineEdit* m_pHostEdit;
    QLineEdit* m_pUserEdit;
    QLineEdit* m_pPasswordEdit;
    QPushButton *m_pTestButton;

    QDialogButtonBox* m_pButtonBox;

    void createActions();
    void saveSettings();
    void loadSettings();

signals:

private slots:
    void onTestConnection();
    void accept();

};

#endif // DATABASESETTINGSDIALOG_H
