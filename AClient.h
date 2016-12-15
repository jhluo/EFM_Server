#ifndef ACLIENT_H
#define ACLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDateTime>
#include <QTextEdit>
#include <QHostAddress>

class AClient : public QObject
{
    Q_OBJECT

public:

    //possible state of a client
    enum eClientState {
        eOnline,
        eNoData,
        eOffline
    };

    typedef struct {
        QString clientDate;
        double temperature;
        double humidity;
        int nIon;
        int pIon;
        int windDirection;
        double windSpeed;
        double rainfall;
        double pressure;
        int ultraViolet;
        double oxygen;
        double pm1;
        double pm25;
        double pm10;
    } ClientData;

    AClient(QObject *pParent = 0);
    ~AClient();

    void setSocket(QTcpSocket *pSocket);

    void closeClient();

    //we use this to register a dialog that will show client data
    void registerDataViewer(QTextEdit *pTextEdit);
    QTextEdit *getDataViewer() {return m_pDataViewer;}

    //getter functions to fetch information about the client
    int getClientId() const { return m_ClientId; }
    QString getClientIp() const { return m_pSocket->peerAddress().toString(); }
    QString getClientState() const;

    //return time of connection and disconnection
    QString getClientConnectTime() const;
    QString getClientDisconnectTime() const;

    //return how long the client has been up in number of seconds;
    QString getClientUpTime() const;

private:
    void handleData(const QByteArray &newData);

    void writeDataLog(const QString &fileName, const ClientData &data);
    void writeRawLog(const QString &fileName, const QByteArray &rawData);
    bool writeDatabase(const ClientData &data);

    QTcpSocket *m_pSocket;

    QByteArray m_DataBuffer;

    //Time stamp when client was connected and disconnected;
    QDateTime m_TimeOfConnect;
    QDateTime m_TimeOfDisconnect;

    //Use to timeout client when no data is coming
    QTimer *m_pDataStarvedTimer;

    //current state of the client
    eClientState m_ClientState;

    int m_ClientId; //ID number of the client;

    //this can be used to display data info from a client
    QTextEdit *m_pDataViewer;

    //method used in decode to convert bytes into a double
    double convertToDecimal(const QByteArray &highByte, const QByteArray &lowByte);

signals:
    void error(QTcpSocket::SocketError socketerror);
    void bytesSent(const int size);
    void newClientConnected();

    //this signal notified GUI to ouput message
    void outputMessage(const QString &msg);

public slots:
    void sendData(const QString &data);

private slots:
    void onDataReceived();
    void onDataTimeout();
    void onSocketDisconnected();
};

#endif // AClient_H
