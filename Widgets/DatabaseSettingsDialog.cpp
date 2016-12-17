#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QtSql>
#include <QDir>
#include <QMessageBox>

#include "DatabaseSettingsDialog.h"
#include "Misc/AppSettings.h"

DatabaseSettingsDialog::DatabaseSettingsDialog(QWidget *pParent) :
    QDialog(pParent)
{
    setWindowTitle(QString(tr("Database Settings")));
    createActions();
}

void DatabaseSettingsDialog::createActions()
{
    AppSettings settings;

    m_pDBNameEdit = new QLineEdit(this);
    m_pHostEdit = new QLineEdit(this);
    m_pUserEdit = new QLineEdit(this);
    m_pPasswordEdit = new QLineEdit(this);
    m_pPasswordEdit->setEchoMode(QLineEdit::Password);

    m_pTestButton = new QPushButton(QString(tr("Test Connection")), this);
    connect(m_pTestButton, SIGNAL(pressed()), this, SLOT(onTestConnection()));

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel,Qt::Horizontal);
    connect(m_pButtonBox, SIGNAL(accepted()),this, SLOT(accept()));
    connect(m_pButtonBox, SIGNAL(rejected()),this, SLOT(reject()));

    QFormLayout *pLayout = new QFormLayout(this);
    pLayout->addRow(QString(tr("Database Name")), m_pDBNameEdit);
    pLayout->addRow(QString(tr("Host")), m_pHostEdit);
    pLayout->addRow(QString(tr("User")), m_pUserEdit);
    pLayout->addRow(QString(tr("Password")), m_pPasswordEdit);
    pLayout->addRow(m_pTestButton);
    pLayout->addRow(m_pButtonBox);

    setLayout(pLayout);

    loadSettings();
}


void DatabaseSettingsDialog::saveSettings()
{
    AppSettings appSet;

    appSet.writeDatabaseSettings("DbName", m_pDBNameEdit->text());
    appSet.writeDatabaseSettings("host", m_pHostEdit->text());
    appSet.writeDatabaseSettings("user", m_pUserEdit->text());
    appSet.writeDatabaseSettings("password", m_pPasswordEdit->text());
}

void DatabaseSettingsDialog::loadSettings()
{
    AppSettings appSet;

    m_pDBNameEdit->setText(appSet.readDatabaseSettings("DbName", "").toString());
    m_pHostEdit->setText(appSet.readDatabaseSettings("host", "").toString());
    m_pUserEdit->setText(appSet.readDatabaseSettings("user", "").toString());
    m_pPasswordEdit->setText(appSet.readDatabaseSettings("password", "").toString());
}

void DatabaseSettingsDialog::onTestConnection()
{
//    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "Dialog");
//    db.setDatabaseName(m_pDBNameEdit->text());
//    db.setHostName(m_pHostEdit->text());
//    db.setPort(m_pPortEdit->text().toInt());
//    db.setUserName(m_pUserEdit->text());
//    db.setPassword(m_pPasswordEdit->text());

//    if(!db.open()) {
//        QMessageBox::critical(0, qApp->tr("Cannot open database"),
//            qApp->tr("Unable to establish a database connection.\n"),
//            QMessageBox::Cancel);
//    } else {
//        QMessageBox::information(0, qApp->tr("Connection Succeed"),
//            qApp->tr("Database connection is successful.\n"),
//            QMessageBox::Cancel);
//      db.close();
//    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "dbDialog");
//    //db.setHostName(m_pHostEdit->text());
    QString dsn = QString("Driver={sql server};server=%1;database=%2;uid=%3;pwd=%4;")
            .arg(m_pHostEdit->text())
            .arg(m_pDBNameEdit->text())
            .arg(m_pUserEdit->text())
            .arg(m_pPasswordEdit->text());

    //QString dsn = QString("Driver={sql server};server=192.168.2.104;database=BJ_epex_DC_2016;uid=sa;pwd=EpEx1234!@#$;");
   // QString dsn = QString("Driver={sql server};server=192.168.2.100;database=BJ_epex_DC_2016;uid=sa;pwd=asdf123;");
    //QString dsn = QString("Driver={SQL server};server=XIEZHOUBJ-PC\JOESQLSERVER;database=BJ_epex_DC_2016;uid=sa;Port=1433;pwd=asdf123;");
    db.setDatabaseName(dsn);
    if(!db.open()) {
        QMessageBox::critical(0,QString(tr("Database Error:")),db.lastError().text());
    } else {
        QMessageBox::information(0, QString(tr("Connection Succeed")),
            QString(tr("Database connection is successful.\n")),
            QMessageBox::Ok);
        db.close();
    }
}

void DatabaseSettingsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}
