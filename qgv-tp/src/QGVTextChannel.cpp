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
#include "gen/textchannel_adapter.h"

static void
dumpObjectProperties(QObject *obj)
{
    const QMetaObject *mo = obj->metaObject ();
    if (NULL == mo) {
        Q_WARN("NULL meta object");
        return;
    }

    Q_DEBUG("Dumping property info:");
    for (int i = 0; i < mo->propertyCount (); i++) {
        QMetaProperty mp = mo->property (i);
        if (mp.isReadable ()) {
            Q_DEBUG(QString("Type: %1. Name: %2. Value: %3")
                    .arg(mp.typeName()).arg(mp.name())
                    .arg(mp.read(obj).toString()));
        } else {
            Q_DEBUG(QString("Unreadable property. Type: %1. Name: %2.")
                    .arg(mp.typeName()).arg(mp.name()));
        }
    }
}//dumpObjectProperties

QGVTextChannel::QGVTextChannel(const QString &objName, const QString &dest,
                               QObject *parent /*= NULL*/)
: QGVChannel(objName, dest, parent)
{
    m_channelType = ofdT_ChannelType_Text;
    m_interfaces << ofdT_ChannelType_Text << ofdT_Chan_I_Messages;

    //m_channelType = ofdT_ChannelType_StreamedMedia;
    //m_interfaces << ofdT_ChannelType_StreamedMedia;
}//QGVTextChannel::QGVTextChannel

QGVTextChannel::~QGVTextChannel()
{
}//QGVTextChannel::~QGVTextChannel

bool
QGVTextChannel::registerObject()
{
    MessagesAdaptor *ma = new MessagesAdaptor(this);
    TextAdaptor *ta = new TextAdaptor(this);
    bool rv = false;

    do {
        if (NULL == ma) {
            Q_WARN("Couldn't allocate Messages adapter");
            break;
        }
        if (NULL == ta) {
            Q_WARN("Couldn't allocate Text adapter");
            break;
        }

        rv = QGVChannel::registerObject ();
    } while (0);

    if (!rv) {
        if (NULL != ta) {
            delete ta;
        }
        if (NULL != ma) {
            delete ma;
        }
        Q_WARN("Failed to register Text channel");
    } else {
        Q_DEBUG("Text channel object registered. Deferring new channel signal");
        QTimer::singleShot (1, this, SLOT(newChannelTimeout()));
    }

    return (rv);
}//QGVTextChannel::registerObject

/*
 * This single shot QTimer slot is to ensure that the new channel signal is
 * emitted only after the handler for EnsureChannel or CreateChannel has
 * returned. This is required according to the spec.
 */
void
QGVTextChannel::newChannelTimeout()
{
    Q_DEBUG("Time to tell the world about this new channel");
    QDBusObjectPath oP(m_dbusObjectPath);
    emit pingNewChannel (oP, m_channelType, 0, 0, true);
}//QGVTextChannel::newChannelTimeout

void
QGVTextChannel::AcknowledgePendingMessages(const Qt_Type_au & /*IDs*/)
{
    Q_DEBUG("Here");
}//QGVTextChannel::AcknowledgePendingMessages

Qt_Type_au
QGVTextChannel::GetMessageTypes()
{
    Q_DEBUG("Returning Message Type = [Normal]");

    Qt_Type_au rv;
    rv << CTMT_Normal;
    return rv;
}//QGVTextChannel::GetMessageTypes

Qt_Type_a_uuuuus
QGVTextChannel::ListPendingMessages(bool /*Clear*/)
{
    Qt_Type_a_uuuuus rv;
    return rv;
}//QGVTextChannel::ListPendingMessages

void
QGVTextChannel::Send(uint Type, const QString &Text)
{
    if (CTT_Normal != Type) {
        sendErrorReply (ofdT_Err_InvalidArgument,"Message type not recognized");
        Q_WARN("Message type not recognized");
        return;
    }

    QDBusInterface iface("org.QGVDial.APIServer",
                         "/org/QGVDial/TextServer",
                         "",
                         QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        sendErrorReply(ofdT_Err_NotAvailable,
                       "qgvtp - QGVDial text interface is not ready");
        Q_WARN("QGVDial text interface is not ready");
        return;
    }

    QStringList listNumbers;
    listNumbers += m_destination;
    iface.call("Text", listNumbers, Text);

    Q_DEBUG("Text sent!");
}//QGVTextChannel::Send

uint
QGVTextChannel::deliveryReportingSupport() const
{
    Q_DEBUG("No delivery reporting support");
    return CDRS_None;
}//QGVTextChannel::deliveryReportingSupport

uint
QGVTextChannel::messagePartSupportFlags() const
{
    Q_DEBUG("No attachments permitted");
    return MPS_No_Attachments;
}//QGVTextChannel::messagePartSupportFlags

Qt_Type_a_a_dict_sv
QGVTextChannel::pendingMessages() const
{
    Q_DEBUG("No pending messages");
    Qt_Type_a_a_dict_sv rv;
    return rv;
}//QGVTextChannel::pendingMessages

QStringList
QGVTextChannel::supportedContentTypes() const
{
    Q_DEBUG("Supported content: text/plain");
    QStringList rv;
    rv << "text/plain";
    return rv;
}//QGVTextChannel::supportedContentTypes

Qt_Type_dict_uv
QGVTextChannel::GetPendingMessageContent(uint /*Message_ID*/,
                                         const Qt_Type_au & /*Parts*/)
{
    Q_DEBUG("No pending message content");
    Qt_Type_dict_uv rv;
    return rv;
}//QGVTextChannel::GetPendingMessageContent

void
QGVTextChannel::SendMessage(const Qt_Type_a_dict_sv &Message, uint Flags)
{
    if (MSF_None != Flags) {
        sendErrorReply (ofdT_Err_InvalidArgument, "Unsupported flags");
        return;
    }

    for (int i = 0; i < Message.count (); i++) {
        QVariantMap oneMap = Message.at (i);

        Q_DEBUG(QString("Map %1").arg(i));
        QStringList keys = oneMap.keys ();
        foreach (QString key, keys) {
            Q_DEBUG(QString("[%1] = %2").arg(key).arg(oneMap[key].toString()));
        }
    }

/*
        QDBusInterface iface("org.QGVDial.APIServer", "/org/QGVDial/TextServer",
                             "", QDBusConnection::sessionBus());
        if (!iface.isValid()) {
            sendErrorReply(ofdT_Err_NotAvailable,
                           "qgvtp - QGVDial text interface is not ready");
            Q_WARN("QGVDial text interface is not ready");
            return false;
        }

        QStringList listNumbers;
        listNumbers += strNum;
        iface.call("TextWithoutData", listNumbers);

        Q_DEBUG("Text initiated successfully");
        sendErrorReply (ofdT_Err_NetworkError, "Channel created successfully");
        success = true;
*/
}//QGVTextChannel::SendMessage
