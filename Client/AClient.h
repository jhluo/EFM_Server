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
    virtual ~AClient();

    //void setSocket(QTcpSocket *pSocket);
    void setDataSource(QIODevice *pInputDevice, const eClientType type);

    QSerialPort *getClientSerialPort();

    //we use this to register a dialog that will show client data
    void registerDataViewer(QTextEdit *pTextEdit);
    QTextEdit *getDataViewer() {return m_pDataViewer;}

    //getter functions to fetch information about the client
    //set displayed client ID only, will not change the ID in the device itself
    void setClientId(const QString &id) {m_ClientId = id;}
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

    inline bool operator==(const AClient &rhs){
        return (this->getClientId()==rhs.getClientId()
                && this->getClientAddress() == rhs.getClientAddress()
                && this->getClientState() == rhs.getClientState() );
    }

protected:
    QString m_ClientId; //ID number of the client;

    //Time stamp when client was connected and disconnected;
    QDateTime m_TimeOfConnect;
    QDateTime m_TimeOfDisconnect;

    //Use to timeout client when no data is coming
    QTimer *m_pDataStarvedTimer;

    //current state of the client
    eClientState m_ClientState;

private:
    void handleData(const QByteArray &newData);

    void writeDataLog(const QString &fileName, const ClientData &data);
    void writeRawLog(const QString &fileName, const QByteArray &rawData);
    bool writeDatabase(const ClientData &data);

    //QTcpSocket *m_pSocket;
    QIODevice *m_pInputDevice;

    QByteArray m_DataBuffer;

    //type of this client
    eClientType m_ClientType;

    //this can be used to display data info from a client
    QTextEdit *m_pDataViewer;

    //whether we display the client in chart dialog
    bool m_ShowChart;

    //method used in decode to convert bytes into a double
    double convertToDecimal(const QByteArray &highByte, const QByteArray &lowByte);

signals:
    void error(QString err);
    void bytesSent(const int size);
    void clientIDAssigned();
    void clientDisconnected();
    void clientDataChanged();


    //signals chart to draw update, only send negative Ion count for now
    void receivedData(const QDateTime &time, const int nIon);

    //this signal notified GUI to ouput message
    void outputMessage(const QString &msg);

public slots:
    //connect and disconnect client
    Q_INVOKABLE virtual void connectClient();
    Q_INVOKABLE virtual void disconnectClient();

    void sendData(const QString &data);

    //slots that turns a serial port off and on
    void setSerialConnect(const bool on);

private slots:
    void onDataReceived();
    void onDataTimeout();
    void onSocketDisconnected();
};

#endif // AClient_H
