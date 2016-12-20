#ifndef ACLIENT_H
#define ACLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QSerialPort>
#include <QIODevice>
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

    enum eClientType {
        eTcp,
        eSerial,
        eUnknown
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

    //void setSocket(QTcpSocket *pSocket);
    void setInputDevice(QIODevice *pInputDevice, const eClientType type);

    void closeClient();

    //we use this to register a dialog that will show client data
    void registerDataViewer(QTextEdit *pTextEdit);
    QTextEdit *getDataViewer() {return m_pDataViewer;}

    //getter functions to fetch information about the client
    QString getClientId() const { return m_ClientId; }
    QString getClientAddress() const;
    QString getClientState() const;
    eClientType getClientType() const { return m_ClientType; }

    //return time of connection and disconnection
    QDateTime getClientConnectTime() const;
    QString getClientDisconnectTime() const;

    //return how long the client has been up in number of seconds;
    QString getClientUpTime() const;

    //chart
    void setShowChart(const bool enabled);
    bool getShowChart() const {return m_ShowChart;}

private:
    void handleData(const QByteArray &newData);

    void writeDataLog(const QString &fileName, const ClientData &data);
    void writeRawLog(const QString &fileName, const QByteArray &rawData);
    bool writeDatabase(const ClientData &data);

    //QTcpSocket *m_pSocket;
    QIODevice *m_pInputDevice;

    QByteArray m_DataBuffer;

    //Time stamp when client was connected and disconnected;
    QDateTime m_TimeOfConnect;
    QDateTime m_TimeOfDisconnect;

    //Use to timeout client when no data is coming
    QTimer *m_pDataStarvedTimer;

    //current state of the client
    eClientState m_ClientState;

    //type of this client
    eClientType m_ClientType;

    QString m_ClientId; //ID number of the client;

    //this can be used to display data info from a client
    QTextEdit *m_pDataViewer;

    //whether we display the client in chart dialog
    bool m_ShowChart;

    //method used in decode to convert bytes into a double
    double convertToDecimal(const QByteArray &highByte, const QByteArray &lowByte);

signals:
    void error(QTcpSocket::SocketError socketerror);
    void bytesSent(const int size);
    void newClientConnected();
    void clientDataChanged();


    //signals chart to draw update, only send negative Ion count for now
    void receivedData(const QDateTime &time, const int nIon);

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
