/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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
    qDebug ("Mosquitto: Waiting for the thread to terminate");
    this->setQuit ();
    if (isRunning ()) {
        wait(10 * 1000);
    }
    qDebug ("Mosquitto: Thread has terminated");
    mosquitto_destroy(mosq);
}//MqClientThread::~MqClientThread

void
MqClientThread::on_connect (int rc)
{
    if (0 != rc) {
        qWarning() << "Mosquitto: Failed in on_connect. Error =" << rc;
        return;
    }
    qDebug() << "Mosquitto: Connected to" << strHost;

    rc = this->subscribe (NULL, strTopic.toLatin1().constData (), 1);
    if (0 != rc) {
        qWarning() << "Mosquitto: Failed in subscribe. Error =" << rc;
        emit status ("Failed to subscribe to Mosquitto server");
        return;
    }
    qDebug() << "Mosquitto: Subscribed to" << strTopic << "established.";
}//MqClientThread::on_connect

void
MqClientThread::on_message (const struct mosquitto_message *message)
{
    if (0 == message->payloadlen) {
        qDebug("Mosquitto: No payload!");
        return;
    }

    QString strPayload = (char*)message->payload;
    qDebug() << "Mosquitto: topic = " << message->topic
             << ". message = " << strPayload;

    if (strPayload.startsWith ("inbox")) {
        emit status ("Mosquitto: New inbox entry");
        emit sigUpdateInbox();
    }
    if (strPayload.startsWith ("contact")) {
        emit status ("Mosquitto: Contact changes");
        emit sigUpdateContacts ();
    }
}//MqClientThread::on_message

void
MqClientThread::on_disconnect()
{
    qDebug ("Mosquitto: disconnect");
}//MqClientThread::on_disconnect

void
MqClientThread::on_publish(uint16_t /*mid*/)
{
    qDebug ("Mosquitto: publish");
}//MqClientThread::on_publish

void
MqClientThread::on_subscribe(uint16_t /*mid*/, int /*qos_count*/,
                             const uint8_t * /*granted_qos*/)
{
    qDebug ("Mosquitto: Subscribed");
}//MqClientThread::on_subscribe

void
MqClientThread::on_unsubscribe(uint16_t /*mid*/)
{
    qDebug ("Mosquitto: unsubscribed");
}//MqClientThread::on_unsubscribe

void
MqClientThread::on_error()
{
    qDebug ("Mosquitto: error");
}//MqClientThread::on_error

void
MqClientThread::run ()
{
#define MQTHREAD_MAX_RETRIES 5
    int rv, retries = 0;
    qDebug ("Mq thread: Enter thread");

    do {
        if (strHost.length () == 0) {
            qWarning ("Mq thread: Invalid Host");
            break;
        }

        qDebug() << "Mq thread: Attempting to connect to" << strHost;
        rv = this->mq_connect (strHost.toLatin1().constData(), port, 60, false);
        if (0 != rv) {
            qWarning() << "Mq thread: Failed to connect. Error =" << rv;
            emit status ("Failed to connect to Mosquitto server");
            this->sleep(5);
        } else {
            emit status ("Connected to Mosquitto server!");
        }

        while (!bQuit) {
            rv = this->loop (1*1000);
            if (MOSQ_ERR_SUCCESS == rv) {
                retries = 0;
                // In the normal case, continue the loop
            } else if ((MOSQ_ERR_INVAL == rv) ||
                       (MOSQ_ERR_NOMEM == rv)) {
                qWarning() << "Mq thread: Unrecoverable error in loop:" << rv;
                retries++;
                break;
            } else if ((MOSQ_ERR_NO_CONN == rv) ||
                       (MOSQ_ERR_PROTOCOL == rv)) {
                qWarning() << "Mq thread: error=" << rv
                           << "Recoverable hopefully";
                this->sleep (1);
                retries++;
                break;
            } else if (MOSQ_ERR_CONN_LOST == rv) {
                qWarning ("Lost connection to mosquitto server");
                rv = this->mq_connect (strHost.toLatin1().constData(), port, 60, true);
            } else {
                qWarning() << "Mq thread: Other error in loop:" << rv;
                retries++;
                break;
            }
        }

        qDebug ("Mq thread: End Mq loop. Unsubscribe and disconnect");
        this->unsubscribe(NULL, strTopic.toLatin1().constData ());
        this->mq_disconnect ();
        this->loop (100);

        if (retries > MQTHREAD_MAX_RETRIES) {
            qWarning ("MqThread: Too many retires!");
            bQuit = true;
        }
    } while (!bQuit);

    // Ready for the next time
    bQuit = false;
    qDebug ("Mq thread: Exit thread");
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
        qWarning() << "Mosquitto: Failed to set user and pass. Error =" << rv;
    }
}//MqClientThread::setUserPass

void
MqClientThread::setQuit (bool set)
{
    bQuit = set;
    qDebug() << "Mosquitto: Request for quit =" << (bQuit?"True":"False");
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
