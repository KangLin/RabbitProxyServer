//! @author Kang Lin <kl222@126.com>

#include "IceSignalQXmppManager.h"
#include "IceSignalQXmppIq.h"
#include "IceSignalQxmpp.h"
#include "QXmppClient.h"
#include <QDomElement>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logQxmpp)

CIceSignalQXmppManager::CIceSignalQXmppManager(CIceSignalQxmpp *pSignal)
    : m_pSignal(pSignal)
{
}

int CIceSignalQXmppManager::sendPacket(CIceSignalQXmppIq &iq)
{
    if(this->client()->sendPacket(iq))
        return 0;
    qCritical(logQxmpp) << "QXmppCallWebrtcManager::sendPacket";
    return -1;
}

QStringList CIceSignalQXmppManager::discoveryFeatures() const
{
    return QStringList() << CIceSignalQXmppIq::ns();
}

bool CIceSignalQXmppManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() != "iq")
        return false;
    if (!CIceSignalQXmppIq::isIceSignalIq(element))
        return false;
    
    CIceSignalQXmppIq iq;
    iq.parse(element);
    if(iq.type() != QXmppIq::Set)
    {
        qCritical(logQxmpp, "The package is error: type: %d; id: %s; from: %s; to: %s",
                        iq.type(),
                        iq.id().toStdString().c_str(),
                        iq.from().toStdString().c_str(),
                        iq.to().toStdString().c_str());
        QXmppIq ack;
        ack.setId(iq.id());
        ack.setTo(iq.from());
        ack.setType(QXmppIq::Error);
        return true;
    }
    
    if(m_pSignal)
        if(!m_pSignal->proecssIq(iq))
        {
            QXmppIq ack;
            ack.setId(iq.id());
            ack.setTo(iq.from());
            ack.setType(QXmppIq::Error);
            return true;
        }
    
    QXmppIq ack;
    ack.setId(iq.id());
    ack.setTo(iq.from());
    ack.setType(QXmppIq::Result);
    return client()->sendPacket(ack);
}
