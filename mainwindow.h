#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindowWidget;
class TheServer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    //construct user interface
    void setupGui();

    //create menu bar
    void populateMenuBar(QMenuBar *pMenuBar);

    //actions taken when closing the app
    void closeEvent(QCloseEvent *event);

    //save and load application settings from INI
    void saveAppSettings();
    void loadAppSettings();

    //member pointers to necessary objects
    MainWindowWidget *m_pMainWidget;
    TheServer *m_pServer;

    //use to notify user for registration
    QTimer m_NotificationTimer;

public slots:
    bool validateApp(const QString &key);

private slots:
    void onNotificationTimer();
    void openDatabaseSettingsDialog();
    void openServerSettingsDialog();
    void openAboutDialog();
    void openRegistrationDialog();
};

#endif // MAINWINDOW_H
