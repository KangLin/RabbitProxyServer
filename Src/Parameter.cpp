//! @author Kang Lin <kl222@126.com>

#include "Parameter.h"
#include <QDebug>

CParameter::CParameter(QObject *parent) : QObject(parent),
    m_nPort(0)
{
}

CParameter::~CParameter()
{
    qDebug() << "CParameter::~CParameter()";
}
quint16 CParameter::GetPort()
{
    return m_nPort;
}

void CParameter::SetPort(quint16 port)
{
    m_nPort = port;
}

int CParameter::Save(QSettings &set)
{
    set.setValue(Name() + "Port", m_nPort);
    return 0;
}

int CParameter::Load(QSettings &set)
{
    m_nPort = set.value(Name() + "Port", m_nPort).toUInt();
    return 0;
}

QString CParameter::Name()
{
    return "";
}
