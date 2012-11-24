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

#ifndef _QGV_CONNECTION_H_
#define _QGV_CONNECTION_H_

#include "global.h"
#include "shared_data_types.h"

class QGVConnection : public QObject, protected QDBusContext
{
    Q_OBJECT

    Q_PROPERTY(QString m_dbusObjectPath READ getDBusObjectPath)
    Q_PROPERTY(QString m_dbusBusName    READ getDBusBusName)

    Q_PROPERTY(int m_selfHandle
               READ getSelfHandle
               WRITE setSelfHandle)

public:
    enum ConnectionStatus {
        Connected       = 0,
        Connecting      = 1,
        Disconnected    = 2
    };
    enum ConnectionStatusReason {
        NoneSpecified           = 0,
        Requested               = 1,
        NetworkError            = 2,
        AuthenticationFailed    = 3,
        EncryptionError         = 4,
        NameInUse               = 5
        // There are more, but I don't care about them right now.
    };

public:
    // DBus Interface properties for Connection
    Q_PROPERTY(bool HasImmortalHandles  READ hasImmortalHandles)
    bool hasImmortalHandles() const;

    Q_PROPERTY(QStringList  Interfaces  READ GetInterfaces)
    Q_PROPERTY(uint         SelfHandle  READ GetSelfHandle)
    Q_PROPERTY(uint         Status      READ GetStatus)

    // DBus Interface properties for Connection.Interface.Requests
    Q_PROPERTY(Qt_Type_a_o_dict_sv Channels READ channels)
    Qt_Type_a_o_dict_sv channels() const;

    Q_PROPERTY(Qt_Type_a_dict_sv_as RequestableChannelClasses READ requestableChannelClasses)
    Qt_Type_a_dict_sv_as requestableChannelClasses() const;


public Q_SLOTS: // METHODS
////////////////////////////////////////////////////////////////////////////////
// Connection interface methods
    void AddClientInterest(const QStringList &Tokens);
    void Connect();
    void Disconnect();
    QStringList GetInterfaces();
    QString GetProtocol();
    uint GetSelfHandle();
    uint GetStatus();
    void HoldHandles(uint Handle_Type, const Qt_Type_au &Handles);
    QStringList InspectHandles(uint Handle_Type, const Qt_Type_au &Handles);
    Qt_Type_a_osuu ListChannels();
    void ReleaseHandles(uint Handle_Type, const Qt_Type_au &Handles);
    void RemoveClientInterest(const QStringList &Tokens);
    QDBusObjectPath RequestChannel(const QString &Type, uint Handle_Type,
                                   uint Handle, bool Suppress_Handler);
    Qt_Type_au RequestHandles(uint Handle_Type, const QStringList &Identifiers);
////////////////////////////////////////////////////////////////////////////////
// Connection.Request interface methods
    QDBusObjectPath CreateChannel(const QVariantMap &Request, QVariantMap &Properties);
    bool EnsureChannel(const QVariantMap &Request, QDBusObjectPath &Channel, QVariantMap &Properties);

Q_SIGNALS: // SIGNALS
////////////////////////////////////////////////////////////////////////////////
// Connection interface signals
    void ConnectionError(const QString &in0, const QVariantMap &in1);
    void NewChannel(const QDBusObjectPath &in0, const QString &in1, uint in2,
                    uint in3, bool in4);
    void SelfHandleChanged(uint in0);
    void StatusChanged(uint status, uint reason);

////////////////////////////////////////////////////////////////////////////////
// Connection.Request interface signals
    void ChannelClosed(const QDBusObjectPath &in0);
    void NewChannels(const Qt_Type_a_o_dict_sv &in0);

public:
    QGVConnection(const QString &u, const QString &p, QObject *parent = NULL);
    ~QGVConnection();

    void setSelfHandle(uint h);
    int getSelfHandle();

    bool registerObject();
    void unregisterObject();

    QString getDBusObjectPath();
    QString getDBusBusName();

private:
    bool processChannel(const QVariantMap &request);

private:
    uint    m_selfHandle;
    QString m_user, m_pass;
    QString m_dbusObjectPath;
    QString m_dbusBusName;
    bool    m_hasImmortalHandle;

    ConnectionStatus m_connStatus;
};

#endif//_QGV_CONNECTION_H_
