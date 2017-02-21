#include "testserver.h"

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

}

void TestServer::cleanup()
{

}

void TestServer::testStartup()
{
    m_pServer->startServer("127.0.0.1", 5101);
    QVERIFY(m_pServer->isListening());
    QVERIFY(m_pServer->serverAddress().toString()=="127.0.0.1");
    QVERIFY(m_pServer->serverPort()==5101);
    m_pServer->shutdownServer();
    QVERIFY(m_pServer->isListening()==false);
}

void TestServer::testShutdown()
{
    m_pServer->close();
    QVERIFY(m_pServer->isListening()==false);
}

void TestServer::testConnection()
{

}
