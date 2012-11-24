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

#include "QGVTextChannel.h"
#include "gen/channel_adapter.h"
#include "gen/textchannel_adapter.h"

QGVTextChannel::QGVTextChannel(QObject *parent /*= NULL*/)
: QGVChannel(parent)
{
    m_channelType = ofdT_ChannelType_Text;
    m_interfaces << ofdT_ChannelType_Text << ofdT_Chan_I_Messages;

    //m_channelType = ofdT_ChannelType_StreamedMedia;
    //m_interfaces << ofdT_ChannelType_StreamedMedia;
}//QGVTextChannel::QGVTextChannel

QGVTextChannel::~QGVTextChannel()
{
}//QGVTextChannel::~QGVTextChannel

void
QGVTextChannel::AcknowledgePendingMessages(const Qt_Type_au &IDs)
{
}//QGVTextChannel::AcknowledgePendingMessages

Qt_Type_au
QGVTextChannel::GetMessageTypes()
{
    Qt_Type_au rv;
    return rv;
}//QGVTextChannel::GetMessageTypes

Qt_Type_a_uuuuus
QGVTextChannel::ListPendingMessages(bool Clear)
{
    Qt_Type_a_uuuuus rv;
    return rv;
}//QGVTextChannel::ListPendingMessages

void
QGVTextChannel::Send(uint Type, const QString &Text)
{
}//QGVTextChannel::Send

uint
QGVTextChannel::deliveryReportingSupport() const
{
    uint rv(0);
    return rv;
}//QGVTextChannel::deliveryReportingSupport

uint
QGVTextChannel::messagePartSupportFlags() const
{
    uint rv(0);
    return rv;
}//QGVTextChannel::messagePartSupportFlags

Qt_Type_au
QGVTextChannel::messageTypes() const
{
    Qt_Type_au rv;
    return rv;
}//QGVTextChannel::messageTypes

Qt_Type_a_a_dict_sv
QGVTextChannel::pendingMessages() const
{
    Qt_Type_a_a_dict_sv rv;
    return rv;
}//QGVTextChannel::pendingMessages

QStringList
QGVTextChannel::supportedContentTypes() const
{
    QStringList rv;
    return rv;
}//QGVTextChannel::supportedContentTypes

Qt_Type_dict_uv
QGVTextChannel::GetPendingMessageContent(uint Message_ID,
                                         const Qt_Type_au &Parts)
{
    Qt_Type_dict_uv rv;
    return rv;
}//QGVTextChannel::GetPendingMessageContent

void
QGVTextChannel::SendMessage(const Qt_Type_a_dict_sv &Message, uint Flags)
{
}//QGVTextChannel::SendMessage
