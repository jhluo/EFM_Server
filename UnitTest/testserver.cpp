#include "testServer.h"
#include <QTcpSocket>

void TestServer::initTestCase()
{
    qDebug() <<"Init";
    m_pServer = new TheServer();
}

void TestServer::cleanupTestCase()
{
    qDebug() <<"Clean up";
    delete m_pServer;
}

void TestServer::init()
{
    m_pServer->startServer("127.0.0.1", 5101);
}

void TestServer::cleanup()
{
    if(m_pServer->isListening())
        m_pServer->shutdownServer();
}

void TestServer::testStartup()
{
    QVERIFY(m_pServer->isListening());
    QVERIFY(m_pServer->serverAddress().toString()=="127.0.0.1");
    QVERIFY(m_pServer->serverPort()==5101);
}

void TestServer::testShutdown()
{
    m_pServer->close();
    QVERIFY(m_pServer->isListening()==false);
}

void TestServer::testTcpConnections()
{
    QSignalSpy spy(m_pServer, SIGNAL(newConnection()));

    //test 1 socket connections
    QTcpSocket *pTestSocket1 = new QTcpSocket(this);
    pTestSocket1->connectToHost(m_pServer->serverAddress().toString(), m_pServer->serverPort());

    // wait returns true if 1 or more signals was emitted
    QCOMPARE(spy.wait(250), true);
    QCOMPARE(m_pServer->getClientCount(), 1);

    //test 2 socket connections
    QTcpSocket *pTestSocket2 = new QTcpSocket(this);
    pTestSocket2->connectToHost(m_pServer->serverAddress().toString(), m_pServer->serverPort());

    QCOMPARE(spy.wait(250), true);
    QCOMPARE(m_pServer->getClientCount(), 2);

    //disconnect socket 1
    pTestSocket1->disconnectFromHost();
    spy.wait(250);
    QCOMPARE(m_pServer->getClientCount(), 1);

    //connect socket 1 again
    pTestSocket1->connectToHost(m_pServer->serverAddress().toString(), m_pServer->serverPort());

    // wait returns true if 1 or more signals was emitted
    QCOMPARE(spy.wait(250), true);
    QCOMPARE(m_pServer->getClientCount(), 2);

    //shut down server
    m_pServer->shutdownServer();
    spy.wait(1000);
    QCOMPARE(m_pServer->getClientCount(), 0);
}
