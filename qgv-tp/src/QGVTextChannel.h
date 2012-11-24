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

#ifndef _QGV_TEXTCHANNEL_H_
#define _QGV_TEXTCHANNEL_H_

#include "global.h"
#include "shared_data_types.h"

class QGVTextChannel : public QGVChannel
{
    Q_OBJECT

public:
    QGVTextChannel(QObject *parent = NULL);
    virtual ~QGVTextChannel();

////////////////////////////////////////////////////////////////////////////////
// Channel.Type.Text Interface:
public: // PROPERTIES
public Q_SLOTS: // METHODS
    void AcknowledgePendingMessages(const Qt_Type_au &IDs);
    Qt_Type_au GetMessageTypes();
    Qt_Type_a_uuuuus ListPendingMessages(bool Clear);
    void Send(uint Type, const QString &Text);
Q_SIGNALS: // SIGNALS
    void LostMessage();
    void Received();
    void SendError();
    void Sent();

////////////////////////////////////////////////////////////////////////////////
// Channel.Interface.Messages Interface
public: // PROPERTIES
    Q_PROPERTY(uint DeliveryReportingSupport READ deliveryReportingSupport)
    uint deliveryReportingSupport() const;

    Q_PROPERTY(uint MessagePartSupportFlags READ messagePartSupportFlags)
    uint messagePartSupportFlags() const;

    Q_PROPERTY(Qt_Type_au MessageTypes READ messageTypes)
    Qt_Type_au messageTypes() const;

    Q_PROPERTY(Qt_Type_a_a_dict_sv PendingMessages READ pendingMessages)
    Qt_Type_a_a_dict_sv pendingMessages() const;

    Q_PROPERTY(QStringList SupportedContentTypes READ supportedContentTypes)
    QStringList supportedContentTypes() const;

public Q_SLOTS: // METHODS
    Qt_Type_dict_uv GetPendingMessageContent(uint Message_ID, const Qt_Type_au &Parts);
    void SendMessage(const Qt_Type_a_dict_sv &Message, uint Flags);
Q_SIGNALS: // SIGNALS
    void MessageReceived();
    void MessageSent();
    void PendingMessagesRemoved();
////////////////////////////////////////////////////////////////////////////////

};

#endif//_QGV_TEXTCHANNEL_H_
