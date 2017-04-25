/*
qgvnotify is a cross platform Google Voice Notification tool
Copyright (C) 2009-2017  Yuvraaj Kelkar

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

#ifndef MQPUBLISHER_H
#define MQPUBLISHER_H

#include "global.h"
#include <mosquitto.h>
#include <stdint.h>

class MqPublisher : public QObject
{
    Q_OBJECT

public:
    MqPublisher(const QString &name,
                const QString &strServer, int port,
                const QString &strTopic,
                QObject *parent = 0);

public slots:
    void publish(const QByteArray &byPayload);
    void on_mqConnected(int result);
    void on_mqDisonnected();
    void on_mqPublish(uint16_t mid);

private:
    QString m_strName;
    QString m_strServer;
    int m_port;
    QString m_strTopic;
    QByteArray m_byPayload;

    bool bConnected;
    struct mosquitto *mosq;
};

#endif // MQPUBLISHER_H
