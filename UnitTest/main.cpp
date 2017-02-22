#include <QtTest>
#include "testServer.h"
#include "testTcpClient.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    TestServer testServer;
    TestTcpClient testTcpClient;

    // multiple test suites can be ran like this
    return QTest::qExec(&testServer, argc, argv)
           | QTest::qExec(&testTcpClient, argc, argv);
}
