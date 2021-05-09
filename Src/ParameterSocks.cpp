#include "ParameterSocks.h"

CParameterSocks::CParameterSocks(QObject *parent) : CParameter(parent),
    m_bIce(false),
    m_bV4(true),
    m_bV5(true)
{
    SetPort(1080);
}

int CParameterSocks::Save(QSettings &set)
{
    CParameter::Save(set);
    set.setValue(Name() + "Ice", m_bIce);
    set.setValue(Name() + "V4", m_bV4);
    set.setValue(Name() + "V5", m_bV5);
    return 0;
}

int CParameterSocks::Load(QSettings &set)
{
    CParameter::Load(set);
    m_bIce = set.value(Name() + "Ice", m_bIce).toBool();
    m_bV4 = set.value(Name() + "V4", m_bV4).toBool();
    m_bV5 = set.value(Name() + "V5", m_bV5).toBool();
    return 0;
}

QString CParameterSocks::Name()
{
    return "Socks/";
}

bool CParameterSocks::GetIce()
{
    return m_bIce;
}

void CParameterSocks::SetIce(bool ice)
{
    m_bIce = ice;
}

bool CParameterSocks::GetV4()
{
    return m_bV4;
}

void CParameterSocks::SetV4(bool v)
{
    m_bV4 = v;
}

bool CParameterSocks::GetV5()
{
    return m_bV5;
}

void CParameterSocks::SetV5(bool v)
{
    m_bV5 = v;
}
