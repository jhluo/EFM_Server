#include "AboutDialog.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QScrollArea>
#include <QDesktopServices>
#include <QPixmap>
#include <QScrollBar>
#include <QMouseEvent>
#include <QDir>

LogoLabel::LogoLabel(QWidget *pParent) : QLabel(pParent)
{
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
}

LogoLabel::LogoLabel(const QPixmap &pix, QWidget *pParent)
    : QLabel(pParent)
{
    setPixmap(pix);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
}

void LogoLabel::setExternalLink(const QString &link)
{
    m_ExternalLink = link;
}


void LogoLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QUrl link(m_ExternalLink);
        QDesktopServices::openUrl(link);
    } else {
        QLabel::mousePressEvent(event);
    }
}


AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    m_pCompanyLogo(NULL),
    m_pAppInfoLabel(NULL)
{
    setWindowTitle("关于负离子接收服务器");
    //setFixedSize(QSize(520, 400));

    QPixmap appLogo(":/epex_logo.png");
    m_pCompanyLogo = new LogoLabel(appLogo.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation), this);
    m_pCompanyLogo->setExternalLink("http://www.epext.com");

    QString infoStr = appInfo();
    m_pAppInfoLabel = new QLabel(this);
    m_pAppInfoLabel->setText(infoStr);
    m_pAppInfoLabel->setWordWrap(true);

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    pMainLayout->addWidget(m_pCompanyLogo);
    pMainLayout->addWidget(m_pAppInfoLabel);

    setLayout(pMainLayout);

    //make dialog nonresizable
    setFixedSize( sizeHint () );
}


void AboutDialog::onCompanyLogoClicked()
{

}

// Returns the credits string for use in the dialog
QString AboutDialog::appInfo() const
{
    QString text;

    text += QString("<strong>负离子接收服务器 (Ver. %1) </strong>\n\n").arg(APP_VERSION);
    return(text);
}
