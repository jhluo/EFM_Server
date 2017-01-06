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
#include "Data/ClientData.h"

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
        eTcpIp,
        eSerial,
        eUnknown
    };

    enum eClientVersion {
        eVersion1,
        eVersion2,
        eVersion3
    };

    AClient(QObject *pParent = 0);
    virtual ~AClient();

    virtual QIODevice *getInputDevice() {return m_pInputDevice;}

    //we use this to register a dialog that will show client data
    void registerDataViewer(QTextEdit *pTextEdit);
    QTextEdit *getDataViewer() {return m_pDataViewer;}

    //getter functions to fetch information about the client
    //set displayed client ID only, will not change the ID in the device itself
    void setClientId(const QString &id) {m_ClientId = id;}
    QString getClientId() const { return m_ClientId; }
    virtual QString getClientAddress() const {return "";}
    QString getClientState() const;
    eClientType getClientType() const {return m_ClientType;}

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
    void setDataSource(QIODevice *pInputDevice, const eClientType &type);

    QString m_ClientId; //ID number of the client;

    //Time stamp when client was connected and disconnected;
    QDateTime m_TimeOfConnect;
    QDateTime m_TimeOfDisconnect;

    //Use to timeout client when no data is coming
    QTimer *m_pDataStarvedTimer;

    //use to disconnect client after no data is transmitted for a while
    QTimer *m_pClientDisconnectTimer;

    //Command ack timer
    QTimer *m_pCommandAckTimer;
    int m_lastCommandSent;

    //current state of the client
    eClientState m_ClientState;

    //type of client
    eClientType m_ClientType;

private:
    void handleData(const QByteArray &newData);

    void decodeVersion1Data(const QByteArray &dataArray);
    void decodeVersion2Data(const QByteArray &dataArray);
    void decodeVersion3Data(const QByteArray &dataArray);

    void writeDataViewer();
    void writeDataLog(const QString &fileName, const ClientData &data);
    void writeRawLog(const QString &fileName, const QByteArray &rawData);
    bool writeDatabase(const ClientData &data);

    //this is used to apply offset values to incoming data
    QVariant applyOffset(const QString &clientId, const ClientData::eDataId id, const QVariant &value);

    QIODevice *m_pInputDevice;

    QByteArray m_DataBuffer;

    //version of data format for this client
    eClientVersion m_ClientVersion;

    //this can be used to display data info from a client
    QTextEdit *m_pDataViewer;

    //whether we display the client in chart dialog
    bool m_ShowChart;

    ClientData m_ClientData;

    //method used in decode to convert bytes into a double
    double convertToDecimal(const QByteArray &highByte, const QByteArray &lowByte);

signals:
    void error(QString err);
    void bytesSent(const int size);
    void clientAcknowledge(const bool ok);
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

    void sendCommand(const QString &data);

    //slots that turns a serial port off and on
    void setSerialConnect(const bool on);

private slots:
    void onDataReceived();
    void onDataTimeout();
    void onCommandAckTimeout();
};

#endif // AClient_H
