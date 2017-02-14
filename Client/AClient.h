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
#include "Data/CommandHandler.h"

class AClient : public QObject
{
    Q_OBJECT

public:

    //possible state of a client
    enum eClientState {
        eOnline,
        eNoData,
        eOffline,
        eUnknownState,
    };

    enum eClientType {
        eTcpIp,
        eSerial,
        eUnknownType
    };

    enum eClientVersion {
        eVersion1,
        eVersion2,
        eVersion3
    };

    AClient(QObject *pParent = 0);
    virtual ~AClient();

    QIODevice *getInputDevice() {return m_pInputDevice;}

    //we use this to register a dialog that will show client data
    void registerDataViewer(QTextEdit *pTextEdit);
    QTextEdit *getDataViewer() {return m_pDataViewer;}

    //getter functions to fetch information about the client
    //set displayed client ID only, will not change the ID in the device itself
    void setClientId(const QString &id) {m_ClientId = id;}
    QString getClientId() const { return m_ClientId; }
    virtual QString getClientAddress() const = 0;
    eClientState getClientState() const {return m_ClientState;}
    eClientType getClientType() const {return m_ClientType;}

    //return time of connection and disconnection
    QDateTime getClientConnectTime() const {return m_TimeOfConnect;}
    QDateTime getClientDisconnectTime() const {return m_TimeOfDisconnect;}

    //QString getClientDisconnectTime() const;

    //return how long the client has been up in number of seconds;
    QString getClientUpTime() const;

    //whether to show chart
    void setShowChart(const bool enabled);
    bool getShowChart() const {return m_ShowChart;}

    //overloaded to check two clients as equivalent
    inline bool operator==(const AClient &rhs){
        return (this->getClientId()==rhs.getClientId()
                && this->getClientAddress() == rhs.getClientAddress()
                && this->getClientState() == rhs.getClientState() );
    }

protected:
    void setDataSource(QIODevice *pInputDevice, const eClientType &type);

    void handleData(const QByteArray &newData);
    void decodeVersion1Data(const QByteArray &dataArray);
    void decodeVersion2Data(const QByteArray &dataArray);
    void decodeVersion3Data(const QByteArray &dataArray);

    void writeDataViewer();
    void writeDataLog(const QString &fileName);
    void writeRawLog(const QString &fileName, const QByteArray &rawData);
    bool writeDatabase();

    //this is used to apply offset values to incoming data
    QVariant applyOffset(const QString &clientId, const ClientData::eDataId id, const QVariant &value);

    //data input channel
    QIODevice *m_pInputDevice;

    //type of client
    eClientType m_ClientType;

    QString m_ClientId; //ID number of the client;

    //current state of the client
    eClientState m_ClientState;

    //version of data format for this client
    eClientVersion m_ClientVersion;

    //container of client data
    ClientData *m_pClientData;

    //object that take care of command communications
    CommandHandler *m_pCommandHandler;

    //Time stamp when client was connected and disconnected;
    QDateTime m_TimeOfConnect;
    QDateTime m_TimeOfDisconnect;

    //Use to timeout client when no data is coming
    QTimer *m_pDataTimer;

    //this can be used to display data info from a client
    QTextEdit *m_pDataViewer;

    //whether we display the client in chart dialog
    bool m_ShowChart;

    //method used in decode to convert bytes into a double
    double convertToDecimal(const QByteArray &highByte, const QByteArray &lowByte);

signals:
    void error(QString err);
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
    Q_INVOKABLE virtual void connectClient()=0;
    Q_INVOKABLE virtual void disconnectClient()=0;

    //send client commands
    void sendCommand(const QString &data);

    //slots that turns a serial port off and on
    void setSerialConnect(const bool on);

private slots:
    void onDataReceived();
    void onDataTimeout();
};

#endif // AClient_H
