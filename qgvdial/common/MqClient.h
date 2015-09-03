/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2015  Yuvraaj Kelkar

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

#ifndef _MQCLIENT_H_
#define _MQCLIENT_H_

#include "mosquittopp.h"
#include "global.h"

class MqClient : public QObject, public mosqpp::mosquittopp
{
    Q_OBJECT

public:
    explicit MqClient(QObject *parent = NULL,
                      const char *id = NULL,
                      bool clean_session = true);
    ~MqClient();

    void setupClient(QThread *thread,
                     const QString &host,
                     const int port = 1883,
                     const int keepalive = 60);

public slots:
    virtual void stopWork(bool wait = true);

// State machine work functions
private slots:
    void startWork();
    void reinitMq(void);
    void reinitConnection(void);
    void doLimbo(void);
    void doWorkLoop(void);

// State machine transitions
signals:
    void beginConnect(void);
    void connectSuccess(void);
    void connectFailure(void);
    void sigStopWork(void);

// Internal signals
signals:
    void sigConnect(int rc);
    void sigDisconnect(int rc);
    void sigPublish(int mid);
    void sigMessage(const struct mosquitto_message *message);
    void sigSubscribe(int mid, int qos_count, const int *granted_qos);
    void sigUnsubscribe(int mid);
    void sigLog(int level, const char *str);
    void sigError();

// Callbacks from mosqpp::mosquittopp
protected:
    virtual void on_connect(int rc);
    virtual void on_disconnect(int rc);
    virtual void on_publish(int mid);
    virtual void on_message(const struct mosquitto_message *msg);
    virtual void on_subscribe(int mid, int qos_count, const int *granted_qos);
    virtual void on_unsubscribe(int mid);
    virtual void on_log(int level, const char *str);
    virtual void on_error();

// Override these to provide your own implementation
protected:
    virtual quint32 getUptimePeriod(void) { return m_workUptimePeriod; }
    virtual quint32 getDowntimePeriod(void) { return m_workDowntimePeriod; }
    virtual quint32 getLimboPeriod(void) { return m_workLimboPeriod; }

protected:
    bool recreateSm(void);
    bool recreateTimer(void);

protected:
    QThread        *m_thread;
    QString         m_host;
    int             m_port;
    int             m_keepalive;

    QStateMachine  *m_sm;
    QTimer         *m_workTimer;

    quint32         m_workUptimePeriod;
    quint32         m_workDowntimePeriod;
    quint32         m_workLimboPeriod;
};

#endif//_MQCLIENT_H_
