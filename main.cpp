#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //translation
    QTranslator translator;
    translator.load("EFMServer_ch");
    a.installTranslator(&translator);

    MainWindow w;
    w.show();

    return a.exec();
}
