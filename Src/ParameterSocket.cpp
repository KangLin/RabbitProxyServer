#include "ParameterSocket.h"

CParameterSocket::CParameterSocket(QObject *parent) : CParameter(parent),
    m_bIce(false),
    m_bV4(true),
    m_bV5(true)
{
    SetPort(1080);
}

int CParameterSocket::Save(QSettings &set)
{
    CParameter::Save(set);
    set.setValue("Socket/Ice", m_bIce);
    set.setValue("Socket/V4", m_bV4);
    set.setValue("Socket/V5", m_bV5);
    return 0;
}

int CParameterSocket::Load(QSettings &set)
{
    CParameter::Load(set);
    m_bIce = set.value("Socket/Ice", m_bIce).toBool();
    m_bV4 = set.value("Socket/V4", m_bV4).toBool();
    m_bV5 = set.value("Socket/V5", m_bV5).toBool();
    return 0;
}

bool CParameterSocket::GetIce()
{
    return m_bIce;
}

void CParameterSocket::SetIce(bool ice)
{
    m_bIce = ice;
}

bool CParameterSocket::GetV4()
{
    return m_bV4;
}

void CParameterSocket::SetV4(bool v)
{
    m_bV4 = v;
}

bool CParameterSocket::GetV5()
{
    return m_bV5;
}

void CParameterSocket::SetV5(bool v)
{
    m_bV5 = v;
}
