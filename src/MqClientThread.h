#ifndef MQCLIENTTHREAD_H
#define MQCLIENTTHREAD_H

#include "global.h"
#include <mosquitto.h>

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class MqClientThread : public QThread
{
    Q_OBJECT
public:
    MqClientThread(const char *name, QObject *parent = 0);
    ~MqClientThread();

    void setSettings(bool bEnable, const QString &host, int port = 0,
                     const QString &topic = "gv_notify");
    void setUserPass(const QString &user, const QString &pass);
    void setQuit(bool set = true);

public:
    void on_connect(int rc);
    void on_message(const struct mosquitto_message *message);
    void on_disconnect();
    void on_publish(uint16_t mid);
    void on_subscribe(uint16_t mid, int qos_count, const uint8_t *granted_qos);
    void on_unsubscribe(uint16_t mid);
    void on_error();

signals:
    void sigUpdateInbox();
    void sigUpdateContacts();
    void status (const QString &strText, int timeout = 3000);

private:
    void run ();
    int subscribe(uint16_t *mid, const char *sub, int qos=0);
    int unsubscribe(uint16_t *mid, const char *sub);
    int mq_connect(const char *host, int port=1883, int keepalive=60,
                   bool clean_session = true);
    int mq_disconnect();
    int loop(int timeout=-1);

private:
    bool    bQuit;
    QString strHost, strTopic, strUser, strPass;
    int     port;

    struct mosquitto *mosq;
};

#endif // MQCLIENTTHREAD_H
