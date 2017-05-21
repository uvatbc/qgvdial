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

#ifndef LIBSERVERINFO_H
#define LIBSERVERINFO_H

#include "global.h"
#include <QObject>  // S^1 :/

class IMainWindow;
class LibServerInfo : public QObject
{
    Q_OBJECT
public:
    explicit LibServerInfo(IMainWindow *parent = NULL);

    void getInfo(void);

private slots:
    void onGotSrvInfo(bool success, const QByteArray &response,
                      QNetworkReply *reply, void *ctx);

private:
    bool parseSrvInfo(const QString &json);

signals:
    void done(bool success);

public:
    QString m_userInfoHost;
    quint16 m_userInfoPort;
    QString m_userInfoTopic;
};

#endif // LIBSERVERINFO_H
