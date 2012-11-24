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

QGVChannel::QGVChannel(QObject *parent /*= NULL*/)
: QObject(parent)
{
}//QGVChannel::QGVChannel

QGVChannel::~QGVChannel()
{
}//QGVChannel::~QGVChannel

void
QGVChannel::Close()
{
}//QGVChannel::Close

QString
QGVChannel::GetChannelType()
{
    QString rv;
    return rv;
}//QGVChannel::GetChannelType

uint
QGVChannel::GetHandle(uint &Target_Handle)
{
    uint rv(0);
    return rv;
}//QGVChannel::GetHandle

QStringList
QGVChannel::GetInterfaces()
{
    QStringList rv;
    return rv;
}//QGVChannel::GetInterfaces

QString
QGVChannel::channelType() const
{
    QString rv;
    return rv;
}//QGVChannel::channelType

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

QStringList
QGVChannel::interfaces() const
{
    QStringList rv;
    return rv;
}//QGVChannel::interfaces

bool
QGVChannel::requested() const
{
    bool rv(false);
    return rv;
}//QGVChannel::requested

uint
QGVChannel::targetHandle() const
{
    uint rv(0);
    return rv;
}//QGVChannel::targetHandle

uint
QGVChannel::targetHandleType() const
{
    uint rv(0);
    return rv;
}//QGVChannel::targetHandleType

QString
QGVChannel::targetID() const
{
    QString rv;
    return rv;
}//QGVChannel::targetID
