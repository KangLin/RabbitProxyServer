#include "SignalWebSocket.h"
#include "RabbitCommonLog.h"

CSignalWebSocket::CSignalWebSocket(QObject *parent) : CSignal(parent)
{
}

int CSignalWebSocket::Write(const char *buf, int nLen)
{
    bool bRet = m_webSocket->send(std::string(buf, nLen));
    if(bRet)
        return 0;
    return -1;
}

int CSignalWebSocket::Read(char *buf, int nLen)
{
    int n = nLen;
    if(m_Data.size() < nLen)
        n = m_Data.size();
    memcpy(buf, &m_Data, n);
    return n;
}


int CSignalWebSocket::Open(const std::string &szUrl)
{
    m_webSocket = std::make_shared<rtc::WebSocket>();
    if(!m_webSocket) return -1;
    m_webSocket->onOpen([]() {
        LOG_MODEL_DEBUG("PeerConnecterIce", "WebSocket is open");
    });
    m_webSocket->onError([](std::string s){
        LOG_MODEL_DEBUG("PeerConnecterIce", "WebSocket is error");
    });
    m_webSocket->onClosed([](){
        LOG_MODEL_DEBUG("PeerConnecterIce", "WebSocket is close");
    });
    m_webSocket->onMessage([this](std::variant<rtc::binary, std::string> data){
        this->m_Data = std::get<rtc::binary>(data);
        //TODO:
    });
    m_webSocket->open(szUrl);
    m_szUrl = szUrl;
    return 0;
}

int CSignalWebSocket::Close()
{
    if(!m_webSocket) return -1;
    m_webSocket->close();
    return 0;
}

bool CSignalWebSocket::IsOpen()
{
    if(m_webSocket)
        return m_webSocket->isOpen();
    return false;
}