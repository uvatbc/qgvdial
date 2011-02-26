#ifndef MQCLIENTTHREAD_H
#define MQCLIENTTHREAD_H

#include "global.h"
#include <mosquittopp.h>

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class MqClientThread : public QThread, protected mosquittopp
{
    Q_OBJECT
public:
    MqClientThread(const char *name, QObject *parent = 0);

    void setSettings(bool bEnable, const QString &host, int port = 0);
    void setUserPass(const QString &user, const QString &pass);
    void setQuit(bool set = true);

signals:
    void sigUpdateInbox();

private:
    void run ();
    void on_connect(int rc);
    void on_message(const struct mosquitto_message *message);

private:
    bool    bQuit;
    QString strHost, strUser, strPass;
    int     port;
};

#endif // MQCLIENTTHREAD_H
