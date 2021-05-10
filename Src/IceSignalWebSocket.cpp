//! @author Kang Lin(kl222@126.com)

#include "IceSignalWebSocket.h"

#include <nlohmann/json.hpp>
#include "RabbitCommonLog.h"

CIceSignalWebSocket::CIceSignalWebSocket(QObject *parent) : CIceSignal(parent)
{
    m_webSocket = std::make_shared<rtc::WebSocket>();
}

int CIceSignalWebSocket::Write(const char *buf, int nLen)
{
    bool bRet = 0;
    if(m_webSocket)
    {
        bRet = m_webSocket->send(std::string(buf, nLen));
        if(bRet)
            return nLen;
    }
    return -1;
}

int CIceSignalWebSocket::Read(char *buf, int nLen)
{
    int n = nLen;
    if(static_cast<int>(m_Data.size()) < nLen)
        n = m_Data.size();
    if(n >= 0)
        memcpy(buf, &m_Data, n);
    else
        return -1;
    return n;
}

int CIceSignalWebSocket::Open(const std::string &szServer, quint16 nPort,
                           const std::string &user, const std::string &password)
{
    Q_UNUSED(password);
    std::string szUrl;
    std::string wsPrefix = "ws://";

    const std::string url = wsPrefix + szServer + ":" +
                       std::to_string(nPort) + "/" + user;
    LOG_MODEL_DEBUG("SignalWebsocket", "Url is %s", url.c_str());
    return Open(url);
}

int CIceSignalWebSocket::Open(const std::string &szUrl)
{
    if(!m_webSocket)
        m_webSocket = std::make_shared<rtc::WebSocket>();
    if(!m_webSocket) return -1;

    m_webSocket->onOpen([this]() {
        LOG_MODEL_DEBUG("SignalWebSocket", "WebSocket is open");
        emit sigConnected();
    });
    m_webSocket->onError([this](std::string szErr) {
        LOG_MODEL_DEBUG("SignalWebSocket", "WebSocket is error");
        emit sigError(-1, szErr.c_str());
    });
    m_webSocket->onClosed([this]() {
        LOG_MODEL_DEBUG("SignalWebSocket", "WebSocket is close");
        emit sigDisconnected();
    });
    m_webSocket->onMessage([this](std::variant<rtc::binary, std::string> data) {
        this->m_Data = std::get<rtc::binary>(data);
        emit sigReadyRead();

        if (!std::holds_alternative<std::string>(data))
            return;

        nlohmann::json message = nlohmann::json::parse(std::get<std::string>(data));

        auto it = message.find("id");
        if (it == message.end())
            return;
        std::string id = it->get<std::string>();

        it = message.find("type");
        if (it == message.end())
            return;
        std::string type = it->get<std::string>();

        if (type == "offer" || type == "answer") {
            auto sdp = message["description"].get<std::string>();
            emit sigDescription(id.c_str(), rtc::Description(sdp, type));
        } else if (type == "candidate") {
            auto sdp = message["candidate"].get<std::string>();
            auto mid = message["mid"].get<std::string>();
            emit sigCandiate(id.c_str(), rtc::Candidate(sdp, mid));
        }
    });
    try {
        m_webSocket->open(szUrl);
    }  catch (std::exception &e) {
        LOG_MODEL_ERROR("SignalWebsocket",
                        "Open websocket fail: %s; %s",
                        e.what(), szUrl.c_str());
        return -1;
    }

    m_szUrl = szUrl;
    return 0;
}

int CIceSignalWebSocket::Close()
{
    if(!m_webSocket) return -1;
    m_webSocket->close();
    return 0;
}

bool CIceSignalWebSocket::IsOpen()
{
    if(m_webSocket)
        return m_webSocket->isOpen();
    return false;
}

int CIceSignalWebSocket::SendCandiate(const QString& user,
                                      const rtc::Candidate &candidate)
{
    nlohmann::json message = {{"id", user.toStdString()},
                    {"type", "candidate"},
                    {"candidate", std::string(candidate)},
                    {"mid", candidate.mid()}};
    std::string m = message.dump();
    return Write(m.c_str(), m.size());
}

int CIceSignalWebSocket::SendDescription(const QString& user,
                                         const rtc::Description &description)
{
    nlohmann::json message = {
        {"id", user.toStdString()},
        {"type", description.typeString()},
        {"description", std::string(description)}};

    std::string m = message.dump();
    return Write(m.c_str(), m.size());
}
