#include "testTcpClient.h"

void TestTcpClient::initTestCase()
{
    m_pServer = new TheServer(this);
    m_pServer->startServer("127.0.0.1", 5101);

    m_pClientSocket = new QTcpSocket(this);
}

void TestTcpClient::cleanupTestCase()
{
    m_pServer->shutdownServer();
}

void TestTcpClient::init()
{
    m_pClientSocket->connectToHost(m_pServer->serverAddress().toString(), m_pServer->serverPort());
    QTest::qWait(250);
}

void TestTcpClient::cleanup()
{
    m_pClientSocket->disconnectFromHost();
    QTest::qWait(500);
}

void TestTcpClient::testDecodeVer1()
{
    //write a version 1 data string
    QString dataStr = "4A 48 57 54 4f 0B 0C 00 3C 3B 0B 0C 0E 0A 38 15 04 1e 14 02 14 03 12 01 06 0a 0c 15 2b 03 f1 0c 06 14 00 00 f4";
    QStringList bytes = dataStr.split(" ");
    QByteArray data;
    for(int i=0; i<bytes.size(); i++) {
        bool ok;
        char byte = bytes.at(i).toUInt(&ok, 16);
        data.append(byte);
    }

    //send the data
    m_pClientSocket->write(data);
    QTest::qWait(1000);

    //now verify the data having been decoded
    AClient *pClient = m_pServer->getClient(0);
    QCOMPARE(pClient->getClientId(), QString("2828"));

    //verify rest of the data
    bool valid = false;
    double temp = pClient->getClientData(ClientData::eTemperature, valid).toDouble();
    QVERIFY(valid);
    QCOMPARE(temp, 21.04);

    valid = false;
    double humidity = pClient->getClientData(ClientData::eHumidity, valid).toDouble();
    QVERIFY(valid);
    QCOMPARE(humidity, 30.2);

    valid = false;
    int nIon = pClient->getClientData(ClientData::eNIon, valid).toInt();
    QVERIFY(valid);
    QCOMPARE(nIon, 532);

    valid = false;
    int pIon = pClient->getClientData(ClientData::ePIon, valid).toInt();
    QVERIFY(valid);
    QCOMPARE(pIon, 786);
}

void TestTcpClient::testDecodeVer2()
{
    //verify the client connection has been established
    //QCOMPARE(m_pServer->getClientCount(), 1);
}

void TestTcpClient::testDecodeVer3()
{
    //verify the client connection has been established
    //QCOMPARE(m_pServer->getClientCount(), 1);
}
