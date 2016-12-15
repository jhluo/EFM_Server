#ifndef DATAVIEWER_H
#define DATAVIEWER_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QTextEdit>
#include <QButtonGroup>

class AClient;

class DataViewer : public QDialog
{
    Q_OBJECT

public:
    explicit DataViewer(AClient *pClient, QWidget *parent = 0);
    ~DataViewer();

    QTextEdit *getMessageDisplay() {return m_pMessageDisplay;}

private:
    void createActions();
    void populateDialog();

    //this holds the client it's displaying
    AClient *m_pClient;

    QTextEdit *m_pMessageDisplay;

private slots:

};

#endif // DATAVIEWER_H
