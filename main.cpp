#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //not needed
    qApp->addLibraryPath( qApp->applicationDirPath() + "/sqldrivers");
    qApp->addLibraryPath( qApp->applicationDirPath() + "/plugins/sqldrivers");

    MainWindow w;
    w.show();

    return a.exec();
}
