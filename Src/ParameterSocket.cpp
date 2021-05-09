#include "ParameterSocket.h"

CParameterSocket::CParameterSocket(QObject *parent) : CParameter(parent),
    m_bIce(false)
{
    SetPort(1080);
}

QDataStream& CParameterSocket::Save(QDataStream &d)
{
    CParameter::Save(d);
    d << m_bIce;
    return d;
}

QDataStream& CParameterSocket::Load(QDataStream &d)
{
    CParameter::Load(d);
    d >> m_bIce;
    return d;
}

bool CParameterSocket::GetIce()
{
    return m_bIce;
}

void CParameterSocket::SetIce(bool ice)
{
    m_bIce = ice;
}
