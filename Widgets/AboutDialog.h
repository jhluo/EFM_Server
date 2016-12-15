#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QLabel>

class LogoLabel : public QLabel
{
    Q_OBJECT

public:
    LogoLabel(QWidget *pParent=0);
    LogoLabel(const QPixmap &pix, QWidget *pParent=0);

    void setExternalLink(const QString &link);

protected:
    void mousePressEvent(QMouseEvent *event);


private:
    QString m_ExternalLink;
};

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = 0);

private:
    LogoLabel *m_pCompanyLogo;
    QLabel *m_pAppInfoLabel;

    // Returns all the application text for use in the dialog
    QString appInfo() const;

private slots:
    void onCompanyLogoClicked();
};

#endif // ABOUTDIALOG_H
