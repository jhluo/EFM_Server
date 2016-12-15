#ifndef ActivationDialog_H
#define ActivationDialog_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class ActivationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ActivationDialog(QWidget *parent = 0);

private:
    void createActions();

    QLineEdit *m_pPasswordEdit;
    QPushButton *m_pConfirmButton;

signals:
    void productKeyConfirmed(const QString &key);

public slots:


private slots:
    void onConfirmButtonClicked();
};

#endif // ActivationDialog_H
