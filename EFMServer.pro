#-------------------------------------------------
#
# Project created by QtCreator 2016-11-30T08:46:30
#
#-------------------------------------------------

QT       += widgets core gui sql serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += network charts

TARGET = EFMServer
VERSION = 2.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

TEMPLATE = app

QTPLUGIN += qsqlodbc

RC_ICONS = epex_logo.ico

TRANSLATIONS = EFMServer_ch.ts

SOURCES += main.cpp\
        mainwindow.cpp \
    Widgets/MainWindowWidget.cpp \
    Misc/Logger.cpp \
    Misc/AppSettings.cpp \
    Widgets/DatabaseSettingsDialog.cpp \
    TheServer.cpp \
    AClient.cpp \
    Widgets/DataViewer.cpp \
    Widgets/ServerSettingsDialog.cpp \
    Widgets/AboutDialog.cpp \
    Widgets/MainStatusbar.cpp \
    Widgets/ActivationDialog.cpp \
    Widgets/ClientCommandDialog.cpp \
    Chart/ClientChart.cpp \
    Chart/ChartDialog.cpp \
    Chart/ChartTabWidget.cpp \
    Widgets/SerialSettingsDialog.cpp \
    AClientList.cpp \
    Table/ClientTableModel.cpp \
    Table/ClientTableView.cpp

HEADERS  += mainwindow.h \
    Widgets/MainWindowWidget.h \
    Misc/Logger.h \
    Misc/AppSettings.h \
    Widgets/DatabaseSettingsDialog.h \
    TheServer.h \
    AClient.h \
    Widgets/DataViewer.h \
    Widgets/ServerSettingsDialog.h \
    Widgets/AboutDialog.h \
    Widgets/MainStatusbar.h \
    Widgets/ActivationDialog.h \
    Widgets/ClientCommandDialog.h \
    Chart/ClientChart.h \
    Chart/ChartDialog.h \
    Chart/ChartTabWidget.h \
    Widgets/SerialSettingsDialog.h \
    AClientList.h \
    Table/ClientTableModel.h \
    Table/ClientTableView.h

FORMS    += mainwindow.ui

RESOURCES += \
    resource.qrc
