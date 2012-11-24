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

#ifndef _QGV_CHANNEL_H_
#define _QGV_CHANNEL_H_

#include "global.h"
#include "shared_data_types.h"

class QGVChannel : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    QGVChannel(QObject *parent = NULL);
    virtual ~QGVChannel();

////////////////////////////////////////////////////////////////////////////////
// Channel Interface:
public: // PROPERTIES
    Q_PROPERTY(QString ChannelType READ GetChannelType)

    Q_PROPERTY(uint InitiatorHandle READ initiatorHandle)
    uint initiatorHandle() const;
    Q_PROPERTY(QString InitiatorID READ initiatorID)
    QString initiatorID() const;

    Q_PROPERTY(QStringList Interfaces READ GetInterfaces)

    Q_PROPERTY(bool Requested READ requested)
    bool requested() const;

    Q_PROPERTY(uint TargetHandle READ targetHandle)
    uint targetHandle();

    Q_PROPERTY(uint TargetHandleType READ targetHandleType)
    uint targetHandleType();

    Q_PROPERTY(QString TargetID READ targetID)
    QString targetID() const;

public Q_SLOTS: // METHODS
    void Close();
    QString GetChannelType();
    uint GetHandle(uint &Target_Handle);
    QStringList GetInterfaces();
Q_SIGNALS: // SIGNALS
    void Closed();
////////////////////////////////////////////////////////////////////////////////

protected:
    QString m_channelType;
    QStringList m_interfaces;
};

#endif//_QGV_CHANNEL_H_
