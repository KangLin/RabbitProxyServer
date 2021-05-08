#ifndef CICESIGNAL_H
#define CICESIGNAL_H

#include "Signal.h"
#include "rtc/rtc.hpp"

class CSignalWebSocket : public CSignal
{
    Q_OBJECT
public:
    explicit CSignalWebSocket(QObject *parent = nullptr);
    
    virtual int Open(const std::string &szUrl) override;
    virtual int Close() override;
    virtual bool IsOpen() override;
    virtual int Write(const char* buf, int nLen) override;
    virtual int Read(char* buf, int nLen) override;

private:
    std::shared_ptr<rtc::WebSocket> m_webSocket;
    std::string m_szUrl;
    rtc::binary m_Data;
};

#endif // CICESIGNAL_H
