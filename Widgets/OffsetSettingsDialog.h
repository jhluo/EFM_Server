#ifndef OFFSETSETTINGSDIALOG_H
#define OFFSETSETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>

//forward declaration
class AClient;

class OffsetSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OffsetSettingsDialog(AClient *pClient, QWidget *pParent = 0);

private:
    QLineEdit* m_pBaseEdit;
    QLineEdit* m_pMultiplierEdit;

    QPushButton *m_pSaveButton;

    QDialogButtonBox* m_pButtonBox;

    AClient *m_pClient;

    void createActions();

    void saveSettings();
    void loadSettings();

signals:

private slots:
    void accept();

};

#endif // OFFSETSETTINGSDIALOG_H
