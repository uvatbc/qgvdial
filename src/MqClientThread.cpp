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

#include "MqClientThread.h"

struct mq_class_init {
    mq_class_init() {
        mosquitto_lib_init();
    }
    ~mq_class_init() {
        mosquitto_lib_cleanup();
    }
}mq_class_init_object;

static void
on_connect_wrapper(void *obj, int rc)
{
    class MqClientThread *m = (class MqClientThread *)obj;
    m->on_connect(rc);
}//on_connect_wrapper

static void
on_disconnect_wrapper(void *obj)
{
    class MqClientThread *m = (class MqClientThread *)obj;
    m->on_disconnect();
}//on_disconnect_wrapper

static void
on_publish_wrapper(void *obj, uint16_t mid)
{
    class MqClientThread *m = (class MqClientThread *)obj;
    m->on_publish(mid);
}//on_publish_wrapper

static void
on_message_wrapper(void *obj, const struct mosquitto_message *message)
{
    class MqClientThread *m = (class MqClientThread *)obj;
    m->on_message(message);
}//on_message_wrapper

static void
on_subscribe_wrapper(void *obj, uint16_t mid, int qos_count, const uint8_t *granted_qos)
{
    class MqClientThread *m = (class MqClientThread *)obj;
    m->on_subscribe(mid, qos_count, granted_qos);
}//on_subscribe_wrapper

static void
on_unsubscribe_wrapper(void *obj, uint16_t mid)
{
    class MqClientThread *m = (class MqClientThread *)obj;
    m->on_unsubscribe(mid);
}//on_unsubscribe_wrapper

MqClientThread::MqClientThread (const char *name, QObject *parent)
: QThread(parent)
, bQuit(false)
, strHost ("localhost")
, strTopic ("gv_notify")
{
    mosq = mosquitto_new(name, this);
    mosquitto_connect_callback_set(mosq, on_connect_wrapper);
    mosquitto_disconnect_callback_set(mosq, on_disconnect_wrapper);
    mosquitto_publish_callback_set(mosq, on_publish_wrapper);
    mosquitto_message_callback_set(mosq, on_message_wrapper);
    mosquitto_subscribe_callback_set(mosq, on_subscribe_wrapper);
    mosquitto_unsubscribe_callback_set(mosq, on_unsubscribe_wrapper);
}//MqClientThread::MqClientThread

MqClientThread::~MqClientThread ()
{
    this->setQuit ();
    wait(2 * 1000);
    mosquitto_destroy(mosq);
}//MqClientThread::~MqClientThread

void
MqClientThread::on_connect (int rc)
{
    if (0 != rc) {
        Q_WARN(QString("Mosquitto: Failed in on_connect. Error = %1").arg(rc));
        return;
    }
    Q_DEBUG(QString("Mosquitto: Connected to %1").arg(strHost));

    rc = this->subscribe (NULL, strTopic.toLatin1().constData (), 0);
    if (0 != rc) {
        Q_WARN(QString("Mosquitto: Failed to subscribe. Error = %1").arg(rc));
        emit status ("Failed to subscribe to Mosquitto server");
        return;
    }
    Q_DEBUG(QString("Mosquitto: Subscription %1 established.").arg(strTopic));
}//MqClientThread::on_connect

void
MqClientThread::on_message (const struct mosquitto_message *message)
{
    if (0 == message->payloadlen) {
        Q_DEBUG("Mosquitto: No payload!");
        return;
    }

    QString strPayload = (char*)message->payload;
    Q_DEBUG(QString("Mosquitto: topic \"%1\". message \"%2\"")
                .arg(message->topic, strPayload));

    QStringList arrPayload = strPayload.split (' ');
    if (arrPayload.length () < 1) {
        Q_WARN("Invalid payload");
        return;
    }

    QDateTime dtUpdate = QDateTime::currentDateTime().toUTC();
    if (arrPayload.length () > 1) {
        quint64 inTime_t = arrPayload[1].toInt();
        if (0 != inTime_t) {
            dtUpdate = QDateTime::fromTime_t(inTime_t);
            dtUpdate.setTimeSpec(Qt::UTC);
        }
    }
    dtUpdate = dtUpdate.toLocalTime();

    QString msg;
    if (arrPayload[0].contains ("inbox")) {
        msg = QString("Mosquitto: Inbox changes date %1")
                        .arg (dtUpdate.toString (Qt::ISODate));
        emit sigUpdateInbox(dtUpdate);
    } else if (arrPayload[0].contains ("contact")) {
        msg = QString("Mosquitto: Contact changes date %1")
                        .arg (dtUpdate.toString (Qt::ISODate));
        emit sigUpdateContacts (dtUpdate);
    }

    Q_DEBUG(msg);
}//MqClientThread::on_message

void
MqClientThread::on_disconnect()
{
    Q_DEBUG("Mosquitto: disconnect");
}//MqClientThread::on_disconnect

void
MqClientThread::on_publish(uint16_t /*mid*/)
{
    Q_DEBUG("Mosquitto: publish");
}//MqClientThread::on_publish

void
MqClientThread::on_subscribe(uint16_t /*mid*/, int /*qos_count*/,
                             const uint8_t * /*granted_qos*/)
{
    Q_DEBUG("Mosquitto: Subscribed");
}//MqClientThread::on_subscribe

void
MqClientThread::on_unsubscribe(uint16_t /*mid*/)
{
    Q_DEBUG("Mosquitto: unsubscribed");
}//MqClientThread::on_unsubscribe

void
MqClientThread::on_error()
{
    Q_DEBUG("Mosquitto: error");
}//MqClientThread::on_error

void
MqClientThread::run ()
{
#define MQTHREAD_MAX_RETRIES 5
    int rv, retries = 0;
    bool firstTime = true;
    Q_DEBUG("Mq thread: Enter thread");

    do {
        if (strHost.length () == 0) {
            Q_WARN("Mq thread: Invalid Host");
            break;
        }

        Q_DEBUG(QString("Mq thread: Attempting to connect to host %1")
                    .arg(strHost));
        rv =
        this->mq_connect (strHost.toLatin1().constData(), port, 60, firstTime);
        firstTime = false;
        if (0 != rv) {
            Q_WARN(QString("Mq thread: Failed to connect. Error = %1").arg(rv));
            emit status("Failed to connect to Mosquitto server");
            this->sleep(5);
        } else {
            emit status("Connected to Mosquitto server!");
        }

        while (!bQuit) {
            rv = this->loop (1*1000);
            if (MOSQ_ERR_SUCCESS == rv) {
                retries = 0;
                // In the normal case, continue the loop
            } else if ((MOSQ_ERR_INVAL == rv) ||
                       (MOSQ_ERR_NOMEM == rv)) {
                Q_WARN(QString("Mq thread: Unrecoverable error in loop: %1")
                            .arg(rv));
                retries++;
                break;
            } else if ((MOSQ_ERR_NO_CONN == rv) ||
                       (MOSQ_ERR_PROTOCOL == rv)) {
                Q_WARN(QString("Mq thread: error = %1. Recoverable hopefully")
                            .arg(rv));
                this->sleep (1);
                retries++;
                break;
            } else if (MOSQ_ERR_CONN_LOST == rv) {
                Q_WARN("Lost connection to mosquitto server");
                rv = this->mq_connect (strHost.toLatin1().constData(), port, 60,
                                       true);
                if (MOSQ_ERR_SUCCESS != rv) {
                    Q_WARN(QString("Mq thread: Failed to connect: %1").arg(rv));
                    retries++;
                    break;
                }
            } else {
                Q_WARN(QString("Mq thread: Other error in loop: %1").arg(rv));
                retries++;
                break;
            }
        }

        Q_DEBUG("Mq thread: End Mq loop. Unsubscribe and disconnect");
        this->unsubscribe(NULL, strTopic.toLatin1().constData ());
        this->mq_disconnect ();
        this->loop (100);

        if (retries > MQTHREAD_MAX_RETRIES) {
            Q_WARN("MqThread: Too many retires!");
            bQuit = true;
        }
    } while (!bQuit);

    // Ready for the next time
    bQuit = false;
    Q_DEBUG("Mq thread: Exit thread");
}//MqClientThread::run

void
MqClientThread::setSettings (bool bEnable, const QString &host, int p,
                             const QString &topic)
{
    if (bEnable) {
        strHost = host;
        if (0 == p) {
            p = 1883;
        }
        port = p;
        strTopic = topic;
    } else {
        strHost.clear ();
        port = 0;
        strTopic.clear ();
    }
}//MqClientThread::setSettings

void
MqClientThread::setUserPass (const QString &user, const QString &pass)
{
    strUser = user;
    strPass = pass;
    int rv = mosquitto_username_pw_set(mosq, strUser.toLatin1().constData (),
                                             strPass.toLatin1().constData ());
    if (0 != rv) {
        Q_WARN(QString("Mosquitto: Failed to set user and pass. Error =")
                        .arg(rv));
    }
}//MqClientThread::setUserPass

void
MqClientThread::setQuit (bool set)
{
    bQuit = set;
    Q_DEBUG(QString("Mosquitto: Request for quit = %1").arg(set));
}//MqClientThread::setQuit

int
MqClientThread::loop(int timeout)
{
    return mosquitto_loop(mosq, timeout);
}//MqClientThread::loop

int
MqClientThread::subscribe(uint16_t *mid, const char *sub, int qos)
{
    return mosquitto_subscribe(mosq, mid, sub, qos);
}//MqClientThread::subscribe

int
MqClientThread::unsubscribe(uint16_t *mid, const char *sub)
{
    return mosquitto_unsubscribe(mosq, mid, sub);
}//MqClientThread::unsubscribe

int
MqClientThread::mq_connect(const char *host, int port, int keepalive,
                           bool clean_session)
{
    return mosquitto_connect(mosq, host, port, keepalive, clean_session);
}//MqClientThread::mq_connect

int
MqClientThread::mq_disconnect()
{
    return mosquitto_disconnect(mosq);
}//MqClientThread::mq_disconnect
