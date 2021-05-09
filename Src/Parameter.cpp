/**
 * @author Kang Lin (kl222@126.com)
 */

#include "Parameter.h"

CParameter::CParameter(QObject *parent) : QObject(parent),
    m_nPort(0)
{
}

quint16 CParameter::GetPort()
{
    return m_nPort;
}

void CParameter::SetPort(quint16 port)
{
    m_nPort = port;
}

QDataStream& CParameter::Save(QDataStream &d)
{
    d << m_nPort;
    return d;
}

QDataStream& CParameter::Load(QDataStream &d)
{
    d >> m_nPort;
    return d;
}
