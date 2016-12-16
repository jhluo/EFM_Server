#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Widgets/MainWindowWidget.h"
#include "Widgets/DatabaseSettingsDialog.h"
#include "Widgets/ServerSettingsDialog.h"
#include "Widgets/ActivationDialog.h"
#include "Widgets/AboutDialog.h"
#include "Widgets/MainStatusbar.h"
#include "Misc/AppSettings.h"
#include "TheServer.h"
#include "Misc/Logger.h"
#include <QMessageBox>

#define NOTIFY_TIMER 3600 * 1000 * 2  //every 2 hours
#define TRIAL_PERIOD    14  //14 days

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("电场仪接收服务器");
    m_pServer = new TheServer(this);

    setupGui();
    loadAppSettings();

    //check product registration
    m_NotificationTimer.setInterval(NOTIFY_TIMER);
    connect(&m_NotificationTimer, SIGNAL(timeout()), this, SLOT(onNotificationTimer()));
    m_NotificationTimer.start();

    //we check the product key first
    AppSettings settings;
    QString productKey = settings.readMiscSettings("productKey", "").toString();
    validateApp(productKey);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    //shut down running modules
    m_pServer->shutdownServer();
    Logger::getInstance()->registerSystemDisplay(NULL);

    saveAppSettings();
    QMainWindow::close();
}

bool MainWindow::validateApp(const QString &key)
{
    AppSettings settings;

    //check the key, if key is good, activate the program
    if (key == "asdf123") {
        LOG_SYS(QString("Product is now running on a valid key %1").arg(key));

        m_NotificationTimer.stop();
        //enable all GUI
        QMenuBar *pMenuBar = this->menuBar();
        QList<QAction*> pActionList = pMenuBar->actions();
        foreach(QAction *pAction, pActionList) {
            pAction->setEnabled(true);
        }

        //start server is not already started
        if(!m_pServer->isListening())
            m_pServer->startServer();

        return true;
    } else if (key != ""){
        LOG_SYS("Product key is invalid.");
    }

    //otherwise we check the trial period
    QDateTime appDate = settings.readMiscSettings("appDate", QDateTime()).toDateTime();

    //first time user, set the first time the app was opened
    if(appDate.isNull()) {
        appDate = QDateTime::currentDateTime();
        settings.writeMiscSettings("appDate", appDate);
    }

    //check whether it has been 14 days since the date it was opened
    int days = appDate.daysTo(QDateTime::currentDateTime());
    //shut the guy down if pass trial period
    if (days >= TRIAL_PERIOD) {
        LOG_SYS("Trial period expired.  Shutting down server...");
        //shutdown the server and disable settings
        if(m_pServer->isListening())
            m_pServer->shutdownServer();

        QMenuBar *pMenuBar = this->menuBar();
        QList<QAction*> pActionList = pMenuBar->actions();
        foreach(QAction *pAction, pActionList) {
            if(pAction->text() != "Register..." || pAction->text() != "Help...") {
                pAction->setDisabled(true);
            }
        }
    } else { //if not yet pass trial period, simply display a dialog
        QMessageBox::information(this, "Trial Period",
            QString("You have %1 days remaining on your trial period.\n"
                    "Input your product key in Help->Register...").arg(TRIAL_PERIOD-days),
            QMessageBox::Ok);

        //we can still start the server
        if(!m_pServer->isListening())
            m_pServer->startServer();
    }

    return false;
}

void MainWindow::setupGui()
{
    m_pMainWidget = new MainWindowWidget(m_pServer, this);
    setCentralWidget(m_pMainWidget);

    QMenuBar *pMenuBar = new QMenuBar(this);
    this->setMenuBar(pMenuBar);
    this->menuBar()->show();
    populateMenuBar(pMenuBar);

    MainStatusbar *pStatusBar = new MainStatusbar(this);
    this->setStatusBar(pStatusBar);
}

void MainWindow::populateMenuBar(QMenuBar *pMenuBar)
{
    QMenu *pSettingsMenu = pMenuBar->addMenu("Settings");

    QAction* pDatabaseSettingsAct = new QAction(QString("Database Settings"), this);
    connect(pDatabaseSettingsAct, SIGNAL(triggered()),this,SLOT(openDatabaseSettingsDialog()));
    pSettingsMenu->addAction(pDatabaseSettingsAct);

    QAction* pServerSettingsAct = new QAction(QString("Server Settings"), this);
    connect(pServerSettingsAct, SIGNAL(triggered()),this,SLOT(openServerSettingsDialog()));
    pSettingsMenu->addAction(pServerSettingsAct);

    QMenu *pHelpMenu = pMenuBar->addMenu("Help");

    QAction* pRegistrationAct = new QAction(QString("Register..."), this);
    connect(pRegistrationAct, SIGNAL(triggered()),this,SLOT(openRegistrationDialog()));
    pHelpMenu->addAction(pRegistrationAct);

    QAction* pAboutAct = new QAction(QString("About..."), this);
    connect(pAboutAct, SIGNAL(triggered()),this,SLOT(openAboutDialog()));
    pHelpMenu->addAction(pAboutAct);
}

void MainWindow::onNotificationTimer()
{
    AppSettings settings;
    QString productKey = settings.readMiscSettings("productKey", "").toString();
    validateApp(productKey);
}

void MainWindow::openDatabaseSettingsDialog()
{
    DatabaseSettingsDialog dialog(this);
    dialog.exec();
}

void MainWindow::openServerSettingsDialog()
{
    ServerSettingsDialog dialog(m_pServer, this);
    dialog.exec();
}

void MainWindow::openRegistrationDialog()
{
    ActivationDialog dialog(this);
    connect(&dialog, SIGNAL(productKeyConfirmed(QString)), this, SLOT(validateApp(QString)));
    dialog.exec();
}

void MainWindow::openAboutDialog()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::saveAppSettings()
{
    //save current dimension of app
    AppSettings settings;
    settings.writeMiscSettings("windowGeometry", saveGeometry());
    settings.writeMiscSettings("windowState", saveState());
}


void MainWindow::loadAppSettings()
{
    //load saveDiemension off App
    AppSettings settings;
    QByteArray geometry = settings.readMiscSettings("windowGeometry", QByteArray()).toByteArray();
    if(!geometry.isEmpty())
        restoreGeometry(settings.readMiscSettings("windowGeometry", "").toByteArray());
    else
        resize(1024, 800);

    restoreState(settings.readMiscSettings("windowState", "").toByteArray());
}

