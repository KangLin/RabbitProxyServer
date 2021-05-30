#ifndef CPARAMETERICE_H
#define CPARAMETERICE_H

#include "Parameter.h"

class RABBITPROXY_EXPORT CParameterIce : public CParameter
{
    Q_OBJECT
    Q_PROPERTY(emIceServerClient IsIceServer READ GetIceServerClient WRITE SetIceServerClient)
    Q_PROPERTY(QString PeerUser READ GetPeerUser WRITE SetPeerUser)
    Q_PROPERTY(QString SignalServer READ GetSignalServer WRITE SetSignalServer)
    Q_PROPERTY(quint16 SignalPort READ GetSignalPort WRITE SetSignalPort)
    Q_PROPERTY(QString StunServer READ GetStunServer WRITE SetStunServer)
    Q_PROPERTY(quint16 StunPort READ GetStunPort WRITE SetStunPort)
    Q_PROPERTY(QString TurnServer READ GetTurnServer WRITE SetStunServer)
    Q_PROPERTY(quint16 TurnPort READ GetTurnPort WRITE SetTurnPort)
    Q_PROPERTY(QString SignalUser READ GetSignalUser WRITE SetSignalUser)
    Q_PROPERTY(QString SignalPassord READ GetSignalPassword WRITE SetSignalPassword)
    Q_PROPERTY(QString TurnUser READ GetTurnUser WRITE SetTurnUser)
    Q_PROPERTY(QString TurnPassword READ GetTurnPassword WRITE SetTurnPassword)
#ifdef _DEBUG
    Q_PROPERTY(bool OnePeerConnectionToOneDataChannel
               READ GetOnePeerConnectionToOneDataChannel
               WRITE SetOnePeerConnectionToOneDataChannel)
#endif
    
public:
    CParameterIce(QObject *parent = nullptr);
    virtual ~CParameterIce();

    virtual int Save(QSettings &set);
    virtual int Load(QSettings &set);

    enum class emIceServerClient{
        Server = 0x01,
        Client = 0x02,
        ServerClient = Server|Client
    };
    emIceServerClient GetIceServerClient();
    void SetIceServerClient(emIceServerClient server);
    QString GetPeerUser();
    void SetPeerUser(const QString &user);
    QString GetSignalServer();
    void SetSignalServer(const QString &szServer);
    quint16 GetSignalPort();
    void SetSignalPort(quint16 port);
    QString GetStunServer();
    void SetStunServer(const QString &szServer);
    quint16 GetStunPort();
    void SetStunPort(quint16 port);
    QString GetTurnServer();
    void SetTurnServer(const QString &szServer);
    quint16 GetTurnPort();
    void SetTurnPort(quint16 port);
    QString GetSignalUser();
    void SetSignalUser(const QString& user);
    QString GetSignalPassword();
    void SetSignalPassword(const QString& password);
    QString GetTurnUser();
    void SetTurnUser(const QString& user);
    QString GetTurnPassword();
    void SetTurnPassword(const QString& password);
#ifdef _DEBUG
    bool GetOnePeerConnectionToOneDataChannel();
    void SetOnePeerConnectionToOneDataChannel(bool bOne);
#endif
    
    QString GenerateChannelId();

private:
    // ICE
    emIceServerClient m_eIceServerClient;
    QString m_szPeerUser;
    QString m_szSignalServer;
    quint16 m_nSignalPort;
    QString m_szSignalUser;
    QString m_szSignalPassword;
    QString m_szStunServer;
    quint16 m_nStunPort;
    QString m_szTurnSerer;
    quint16 m_nTurnPort;
    QString m_szTurnUser;
    QString m_szTurnPassword;
#ifdef _DEBUG
    bool m_bOnePeerConnectionToOneDataChannel;
#endif
    quint64 m_nChannelId;
};

#endif // CPARAMETERICE_H
