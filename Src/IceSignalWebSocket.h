//! @author Kang Lin(kl222@126.com)

#ifndef CICESIGNAL_H
#define CICESIGNAL_H

#include "IceSignal.h"
#include "rtc/rtc.hpp"

class CIceSignalWebSocket : public CIceSignal
{
    Q_OBJECT
public:
    explicit CIceSignalWebSocket(QObject *parent = nullptr);
    virtual ~CIceSignalWebSocket();

    virtual int Open(const std::string& szServer, quint16 nPort,
                     const std::string& user, const std::string& password);
    virtual int Open(const std::string &szUrl) override;
    virtual int Close() override;
    virtual bool IsOpen() override;

    virtual int SendDescription(const QString& user,
                                const QString& id,
                                const rtc::Description& description);
    virtual int SendCandiate(const QString& user,
                             const QString& id,
                             const rtc::Candidate& candidate);

    virtual int Write(const char* buf, int nLen) override;
    virtual int Read(char* buf, int nLen) override;

private:
    std::shared_ptr<rtc::WebSocket> m_webSocket;
    std::string m_szUrl;
    rtc::binary m_Data;
};

#endif // CICESIGNAL_H
