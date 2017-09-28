/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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

// Channel Delivery Reporting Support Flags = CDRS Flags
#define CDRS_None                   0
#define CDRS_Receive_Failures       1
#define CDRS_Receive_Successes      2
#define CDRS_Receive_Read           4
#define CDRS_Receive_Deleted        8

// Channel Message Part Support Flags = MPS Flags
#define MPS_No_Attachments          0
#define MPS_One_Attachment          1
#define MPS_Multiple_Attachments    2

// Channel Message Sending Flags
#define MSF_None                    0
#define MSF_Report_Delivery         1
#define MSF_Report_Read             2
#define MSF_Report_Deleted          4

// Channel Text Message Type
enum Channel_Text_Message_Type {
    CTMT_Normal          = 0,
    CTMT_Action          = 1,
    CTMT_Notice          = 2,
    CTMT_Auto_Reply      = 3,
    CTMT_Delivery_Report = 4
};

class QGVTextChannel : public QGVChannel
{
    Q_OBJECT

    enum ChannelTextType {
        CTT_Normal          = 0,
        CTT_Action          = 1,
        CTT_Notice          = 2,
        CTT_Auto_Reply      = 3,
        CTT_Delivery_Report = 4
    };

public:
    QGVTextChannel(const QString &objName, const QString &dest,
                   QObject *parent = NULL);
    virtual ~QGVTextChannel();
    virtual bool registerObject();

private slots:
    void newChannelTimeout();

Q_SIGNALS:
    void pingNewChannel(const QDBusObjectPath &Object_Path,
                        const QString &Channel_Type, uint Handle_Type,
                        uint Handle, bool Suppress_Handler);


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
    void Received(uint ID, uint Timestamp, uint Sender, uint Type, uint Flags, const QString &Text);
    void SendError(uint Error, uint Timestamp, uint Type, const QString &Text);
    void Sent(uint Timestamp, uint Type, const QString &Text);

////////////////////////////////////////////////////////////////////////////////
// Channel.Interface.Messages Interface
public: // PROPERTIES
    Q_PROPERTY(uint DeliveryReportingSupport READ deliveryReportingSupport)
    uint deliveryReportingSupport() const;

    Q_PROPERTY(uint MessagePartSupportFlags READ messagePartSupportFlags)
    uint messagePartSupportFlags() const;

    Q_PROPERTY(Qt_Type_au MessageTypes READ GetMessageTypes)

    Q_PROPERTY(Qt_Type_a_a_dict_sv PendingMessages READ pendingMessages)
    Qt_Type_a_a_dict_sv pendingMessages() const;

    Q_PROPERTY(QStringList SupportedContentTypes READ supportedContentTypes)
    QStringList supportedContentTypes() const;

public Q_SLOTS: // METHODS
    Qt_Type_dict_uv GetPendingMessageContent(uint Message_ID, const Qt_Type_au &Parts);
    void SendMessage(const Qt_Type_a_dict_sv &Message, uint Flags);
Q_SIGNALS: // SIGNALS
    void MessageReceived(const Qt_Type_a_dict_sv &Message);
    void MessageSent(const Qt_Type_a_dict_sv &Content, uint Flags, const QString &Message_Token);
    void PendingMessagesRemoved(const Qt_Type_au &Message_IDs);
////////////////////////////////////////////////////////////////////////////////

};

#endif//_QGV_TEXTCHANNEL_H_
