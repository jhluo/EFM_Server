#pragma once

#include <QtTest>
#include "TheServer.h"

//test TCP server
class TestServer : public QObject {
    Q_OBJECT

private slots:
    // functions executed by QtTest before and after test suite
    void initTestCase();
    void cleanupTestCase();

    // functions executed by QtTest before and after each test
    void init();
    void cleanup();

    void testStartup();
    void testShutdown();

private:
    TheServer *m_pServer;
};
