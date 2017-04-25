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

#include "MqPublisher.h"

struct mq_class_init {
    mq_class_init() {
        mosquitto_lib_init();
    }
    ~mq_class_init() {
        mosquitto_lib_cleanup();
    }
}mq_class_init_object;

void
my_connect_callback(void *obj, int result)
{
    MqPublisher *self = (MqPublisher *)obj;
    self->on_mqConnected(result);
}

void
my_disconnect_callback(void *obj)
{
    MqPublisher *self = (MqPublisher *)obj;
    self->on_mqDisonnected();
}

void
my_publish_callback(void *obj, uint16_t mid)
{
    MqPublisher *self = (MqPublisher *)obj;
    self->on_mqPublish(mid);
}

MqPublisher::MqPublisher(const QString &name,
                         const QString &strServer, int port,
                         const QString &strTopic,
                         QObject *parent)
: QObject(parent)
, m_strName(name)
, m_strServer(strServer)
, m_port(port)
, m_strTopic(strTopic)
, bConnected(true)
{
}//MqPublisher::MqPublisher

void
MqPublisher::publish(const QByteArray &byPayload)
{
    mosq = mosquitto_new(m_strName.toLatin1().data(), this);
    if (NULL == mosq) {
        qWarning("MqPub: Failed to allocate mosquitto context");
        return;
    }
    mosq->userdata = this;

    m_byPayload = byPayload;

    mosquitto_connect_callback_set(mosq, my_connect_callback);
    mosquitto_disconnect_callback_set(mosq, my_disconnect_callback);
    mosquitto_publish_callback_set(mosq, my_publish_callback);

    int rv =
    mosquitto_connect(mosq, m_strServer.toLatin1().data(), m_port, 60, true);
    if (rv) {
        qCritical("MqPub: Failed to connect to Mq Server");
        return;
    }

    while(!mosquitto_loop(mosq, -1, 1) && bConnected);

    mosquitto_destroy(mosq);
}//MqPublisher::publish

void
MqPublisher::on_mqConnected(int result)
{
    if (0 != result) {
        on_mqDisonnected();
        return;
    }

    int mid_sent = 0;
    mosquitto_publish(mosq, &mid_sent,
                      m_strTopic.toLatin1().data(),
                      m_byPayload.length(),
                      (const uint8_t *)m_byPayload.constData(),
                      0,    // QOS: guaranteed delivery. 4 way handshake
                      true);
}//MqPublisher::on_mqConnected

void
MqPublisher::on_mqDisonnected()
{
    bConnected = false;
}//MqPublisher::on_mqDisonnected

void
MqPublisher::on_mqPublish(uint16_t /*mid*/)
{
    mosquitto_disconnect(mosq);
}//MqPublisher::on_mqPublish
