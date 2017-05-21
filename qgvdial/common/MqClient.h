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

    void setupClient(const QString &topic,
                     const QString &host,
                     const int port = 1883,
                     const int keepalive = 60);

public slots:
    void startSubWork();
    void startPubWork(const QByteArray &payload);
    void stopWork();
signals:
    void dataMessage(QByteArray msg);
    void smCompleted(void);

// State machine work functions
private slots:
    void reinitMq(void);
    void reinitConnection(void);
    void doLimbo(void);
    void doSubscribe(void);
    void doPublish(void);
    void doDisconnect(void);
    void doWorkLoop(void);

// State machine transitions
signals:
    void beginConnect(void);
    void connectSuccess(void);
    void connectFailure(void);
    void subscribeSuccess(void);
    void disconnectDone(void);
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

protected:
    bool recreateSubSm(void);
    bool recreatePubSm(void);

// For the notifiers:
private slots:
    void deleteNotifiers(void);
    void onReadActivated(int s);
    void onWriteActivated(int s);
    void onExceptActivated(int s);

protected:
    QSocketNotifier *m_readNotifier;
    QSocketNotifier *m_writeNotifier;
    QSocketNotifier *m_exceptNotifier;

    QString         m_host;
    int             m_port;
    QString         m_topic;
    int             m_keepalive;
    QByteArray      m_byPayload;

    QStateMachine  *m_sm;

    quint32         m_workLimboPeriod;
    quint32         m_eventsInLoop;

    bool            m_quitSet;
};

#endif//_MQCLIENT_H_
