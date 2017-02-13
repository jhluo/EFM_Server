#include <QtTest>
#include "testserver.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    TestServer testServer;
    // multiple test suites can be ran like this
    return QTest::qExec(&testServer, argc, argv);
           //|QTest::qExec(&testParser, argc, argv);
}
