#pragma once

#include <QtTest>
#include <QTcpSocket>
#include "TheServer.h"
#include "Client/TcpClient.h"

//test TCP Client connection
class TestTcpClient : public QObject {
    Q_OBJECT

private slots:
    // functions executed by QtTest before and after test suite
    void initTestCase();
    void cleanupTestCase();

    // functions executed by QtTest before and after each test
    void init();
    void cleanup();

    //Actual test cases
    void testDecodeVer1();
    void testDecodeVer2();
    void testDecodeVer3();

private:
    TheServer *m_pServer;
    QTcpSocket *m_pClientSocket;
};
