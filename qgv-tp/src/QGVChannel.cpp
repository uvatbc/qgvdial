/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

#include "QGVChannel.h"
#include "gen/channel_adapter.h"

QGVChannel::QGVChannel(const QString &objName, const QString &dest,
                       QObject *parent /*= NULL*/)
: QObject(parent)
, m_dbusObjectPath(objName)
, m_destination(dest)
{
    m_interfaces << ofdT_Channel;
}//QGVChannel::QGVChannel

QGVChannel::~QGVChannel()
{
}//QGVChannel::~QGVChannel

bool
QGVChannel::registerObject()
{
    ChannelAdaptor *ca = new ChannelAdaptor(this);
    if (NULL == ca) {
        Q_WARN("Failed to create channel adapter object");
        return false;
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    bool rv = sessionBus.registerObject(m_dbusObjectPath, this);
    if (!rv) {
        Q_WARN(QString("Couldn't register Channel object path %1")
                .arg(m_dbusObjectPath));
        delete ca;
        return false;
    }

    Q_DEBUG(QString("Registered channel object %1").arg(m_dbusObjectPath));

    return true;
}//QGVChannel::registerObject

void
QGVChannel::Close()
{
    Q_DEBUG("Channel close requested");
}//QGVChannel::Close

QString
QGVChannel::GetChannelType()
{
    Q_DEBUG(QString("Channel type = %1").arg(m_channelType));
    return m_channelType;
}//QGVChannel::GetChannelType

uint
QGVChannel::GetHandle(uint &Target_Handle)
{
    Target_Handle = 0;
    return ofdT_HT_None;
}//QGVChannel::GetHandle

QStringList
QGVChannel::GetInterfaces()
{
    Q_DEBUG(QString("Returning interfaces [%1]").arg(m_interfaces.join(", ")));
    return m_interfaces;
}//QGVChannel::GetInterfaces

uint
QGVChannel::initiatorHandle() const
{
    uint rv(0);
    return rv;
}//QGVChannel::initiatorHandle

QString
QGVChannel::initiatorID() const
{
    QString rv;
    return rv;
}//QGVChannel::initiatorID

bool
QGVChannel::requested() const
{
    // Until I support notification of an incoming call or text, requested is
    // ALWAYS true
    return (true);
}//QGVChannel::requested

uint
QGVChannel::targetHandle()
{
    uint Target_Handle(0);
    GetHandle (Target_Handle);
    return Target_Handle;
}//QGVChannel::targetHandle

uint
QGVChannel::targetHandleType()
{
    uint Target_Handle(0);
    return GetHandle (Target_Handle);
}//QGVChannel::targetHandleType

QString
QGVChannel::targetID() const
{
    QString rv;
    return rv;
}//QGVChannel::targetID
