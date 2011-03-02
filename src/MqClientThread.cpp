#include "MqClientThread.h"

MqClientThread::MqClientThread (const char *name, QObject *parent)
: QThread(parent)
, mosquittopp(name)
, bQuit(false)
, strHost ("localhost")
, strTopic ("gv_notify")
{
}//MqClientThread::MqClientThread

void
MqClientThread::on_connect (int rc)
{
    if (0 != rc) {
        qWarning() << "Mosquitto: Failed in on_connect. Error =" << rc;
        return;
    }

    rc = this->subscribe (NULL, strTopic.toLatin1().constData ());
    if (0 != rc) {
        qWarning() << "Mosquitto: Failed in subscribe. Error =" << rc;
        return;
    }
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
MqClientThread::run ()
{
    qDebug ("Mosquitto: Enter thread loop");

    do { // Begin cleanup block (not a loop)
        if (strHost.length () == 0) {
            qWarning ("Mosquitto: Invalid Host");
            break;
        }

        QHostInfo hInfo = QHostInfo::fromName (strHost);
        QString strFirst = hInfo.addresses().first().toString();

        int rv = ((mosquittopp*)this)->connect (strFirst.toLatin1().constData ());
        if (0 != rv) {
            qWarning() << "Mosquitto: Failed to connect. Error =" << rv;
            break;
        }

        while (!bQuit) {
            qDebug ("Mosquitto: Working the Mq loop");
            this->loop (1*1000);
        }

        qDebug ("Mosquitto: End Mq loop. Unsubscribe and disconnect");
        this->unsubscribe(NULL, strTopic.toLatin1().constData ());
        ((mosquittopp*)this)->disconnect ();
        this->loop (100);
    } while (0); // End cleanup block (not a loop)

    // Ready for the next time
    bQuit = false;
    qDebug ("Mosquitto: Exit thread loop");
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
    int rv =
    this->username_pw_set(strUser.toLatin1().constData (),
                          strPass.toLatin1().constData ());
    if (0 != rv) {
        qWarning() << "Mosquitto: Failed to set user and pass. Error =" << rv;
    }
}//MqClientThread::setUserPass

void
MqClientThread::setQuit (bool set)
{
    bQuit = set;
    qDebug() << "Mosquitto: quit = " << (bQuit?"True":"False");
}//MqClientThread::setQuit
