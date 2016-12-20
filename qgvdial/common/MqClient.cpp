/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016  Yuvraaj Kelkar

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

#include "MqClient.h"

#define MQ_LIMBO_MIN    (3 * 1000)
#define MQ_LIMBO_MAX    (60 * 1000)
#define MQ_LIMBO_INC    (3 * 1000)

#define MQ_MAX_LOOPS     10

MqClient::MqClient(QObject *parent, const char *id, bool clean_session)
: QObject(parent)
, mosquittopp(id, clean_session)
, m_readNotifier(NULL)
, m_exceptNotifier(NULL)
, m_sm(NULL)
, m_workLimboPeriod(MQ_LIMBO_MIN)
, m_quitSet(false)
{
}//MqClient::MqClient

MqClient::~MqClient()
{
}//MqClient::~MqClient

void
MqClient::setupClient(const QString &topic,
                      const QString &host,
                      const int port,
                      const int keepalive)
{
    m_topic = topic;
    m_host = host;
    m_port = port;
    m_keepalive = keepalive;
}//MqClient::setupClient

void
MqClient::on_connect(int rc)
{
    m_eventsInLoop++;
    emit sigConnect(rc);

    if (MOSQ_ERR_SUCCESS == rc) {
        Q_DEBUG("Connected");
        emit connectSuccess();
    } else {
        Q_WARN("Connection failure");
        emit connectFailure();
    }
}//MqClient::on_connect

void
MqClient::on_disconnect(int rc)
{
    m_eventsInLoop++;
    Q_DEBUG("Disconnected");
    emit sigDisconnect(rc);
    emit disconnectDone();
}//MqClient::on_disconnect

void
MqClient::on_publish(int mid)
{
    m_eventsInLoop++;
    emit sigPublish(mid);
}//MqClient::on_publish

void
MqClient::on_message(const struct mosquitto_message *msg)
{
    m_eventsInLoop++;
    emit sigMessage(msg);

    QByteArray ba((char*)msg->payload, msg->payloadlen);
    emit dataMessage(ba);
}//MqClient::on_message

void
MqClient::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
    m_eventsInLoop++;
    emit sigSubscribe(mid, qos_count, granted_qos);
    emit subscribeSuccess();
}//MqClient::on_subscribe

void
MqClient::on_unsubscribe(int mid)
{
    m_eventsInLoop++;
    emit sigUnsubscribe(mid);
}//MqClient::on_unsubscribe

void
MqClient::on_log(int level, const char *str)
{
    m_eventsInLoop++;
    emit sigLog(level, str);
}//MqClient::on_log

void
MqClient::on_error()
{
    m_eventsInLoop++;
    emit sigError();
}//MqClient::on_error

void
MqClient::startWork()
{
    if (!recreateSm()) {
        Q_WARN("Failed to create SM");
    }
}//MqClient::startWork

void
MqClient::stopWork()
{
    m_quitSet = true;
    emit sigStopWork();
}//MqClient::stopWork

bool
MqClient::recreateSm(void)
{
    SAFE_DELETE(m_sm);

    m_sm = new QStateMachine(this);

    QState *initialState    = new QState;
    QState *connectingState = new QState;
    QState *limboState      = new QState;
    QState *subscribeState  = new QState;
    QState *workState       = new QState;
    QState *disconnectState = new QState;
    QFinalState *endState = new QFinalState;

    if ((NULL == m_sm) ||
        (NULL == initialState) ||
        (NULL == connectingState) ||
        (NULL == limboState) ||
        (NULL == subscribeState) ||
        (NULL == workState) ||
        (NULL == disconnectState) ||
        (NULL == endState))
    {
        Q_WARN("Some states could not be created!!");
        SAFE_DELETE(endState);
        SAFE_DELETE(disconnectState);
        SAFE_DELETE(workState);
        SAFE_DELETE(subscribeState);
        SAFE_DELETE(limboState);
        SAFE_DELETE(connectingState);
        SAFE_DELETE(initialState);
        SAFE_DELETE(m_sm);
        return false;
    }

    // Begin at init
    QObject::connect(initialState, SIGNAL(entered()),
                     this, SLOT(reinitMq()));
    // Work done in connecting state
    QObject::connect(connectingState, SIGNAL(entered()),
                     this, SLOT(reinitConnection()));
    // Work done in limbo state
    QObject::connect(limboState, SIGNAL(entered()),
                     this, SLOT(doLimbo()));
    // Work done in subscribe state
    QObject::connect(subscribeState, SIGNAL(entered()),
                     this, SLOT(doSubscribe()));
    // Work done in disconnect state
    QObject::connect(disconnectState, SIGNAL(entered()),
                     this, SLOT(doDisconnect()));
    // There is no work to be done in the "workState" :)

#define ADD_TRANSITION(_src, _sig, _dst) \
    (_src)->addTransition(this, SIGNAL(_sig()), (_dst))

    ADD_TRANSITION(   initialState,     beginConnect, connectingState);
    ADD_TRANSITION(connectingState,   connectSuccess, subscribeState);
    ADD_TRANSITION(connectingState,   connectFailure, limboState);
    ADD_TRANSITION(     limboState,     beginConnect, connectingState);
    ADD_TRANSITION( subscribeState, subscribeSuccess, workState);
    ADD_TRANSITION( subscribeState,   connectFailure, limboState);
    ADD_TRANSITION(      workState,   connectFailure, limboState);
    ADD_TRANSITION(      workState,      sigStopWork, disconnectState);
    ADD_TRANSITION(disconnectState,   disconnectDone, endState);

#undef ADD_TRANSITION

    QObject::connect(m_sm, SIGNAL(finished()), this, SIGNAL(smCompleted()));
    QObject::connect(m_sm, SIGNAL(finished()), this, SLOT(deleteLater()));

    m_sm->addState(initialState);
    m_sm->addState(connectingState);
    m_sm->addState(limboState);
    m_sm->addState(subscribeState);
    m_sm->addState(workState);
    m_sm->addState(disconnectState);
    m_sm->addState(endState);
    m_sm->setInitialState(initialState);

    m_sm->start();
    return true;
}//MqClient::recreateSm

void
MqClient::reinitMq(void)
{
    emit beginConnect();
}//MqClient::reinitMq

void
MqClient::deleteNotifiers(void)
{
    if (m_readNotifier) {
        m_readNotifier->deleteLater();
        m_readNotifier = NULL;
    }
    if (m_exceptNotifier) {
        m_exceptNotifier->deleteLater();
        m_exceptNotifier = NULL;
    }
}//MqClient::deleteNotifiers

void
MqClient::reinitConnection(void)
{
    int rv;
    rv = mosqpp::mosquittopp::connect(m_host.toLatin1().constData(),
                                      m_port,
                                      m_keepalive);
    if (MOSQ_ERR_SUCCESS != rv) {
        Q_WARN(QString("Connection attempt failed. rv = %1").arg(rv));
        emit connectFailure();
        return;
    }

    deleteNotifiers();

    m_readNotifier = new QSocketNotifier(this->socket(),
                                           QSocketNotifier::Read,
                                           this);
    m_exceptNotifier = new QSocketNotifier(this->socket(),
                                           QSocketNotifier::Exception,
                                           this);
    if ((NULL == m_readNotifier) ||
        (NULL == m_exceptNotifier))
    {
        Q_WARN("Failed to allocate one of the socket notifiers!");
        deleteNotifiers();
        mosqpp::mosquittopp::disconnect();
        return;
    }

    QObject::connect(m_readNotifier, SIGNAL(activated(int)),
                     this, SLOT(onReadActivated(int)));
    QObject::connect(m_exceptNotifier, SIGNAL(activated(int)),
                     this, SLOT(onExceptActivated(int)));

    Q_DEBUG("Connection attempt started");
    doWorkLoop();
}//MqClient::reinitConnection

void
MqClient::onReadActivated(int /*s*/)
{
    if (m_readNotifier) {
        m_readNotifier->setEnabled(false);
        doWorkLoop();
        m_readNotifier->setEnabled(true);
    }
}//MqClient::onReadActivated

void
MqClient::onExceptActivated(int /*s*/)
{
    if (m_exceptNotifier) {
        m_exceptNotifier->setEnabled(false);
        doWorkLoop();
        m_exceptNotifier->setEnabled(true);
    }
}//MqClient::onExceptActivated

void
MqClient::doLimbo(void)
{
    Q_DEBUG(QString("Connect to mosquitto server '%1' failed. "
                    "Backoff for %2 ms")
                .arg(m_host).arg(m_workLimboPeriod));
    QTimer::singleShot(m_workLimboPeriod, this, SIGNAL(beginConnect()));

    m_workLimboPeriod += MQ_LIMBO_INC;
    if (m_workLimboPeriod >= MQ_LIMBO_MAX) {
        m_workLimboPeriod = MQ_LIMBO_MAX;
    }
}//MqClient::doLimbo

void
MqClient::doSubscribe(void)
{
    Q_DEBUG(QString("Attempting to subscribe to '%1'").arg(m_topic));
    m_workLimboPeriod = MQ_LIMBO_MIN;

    int rv;
    rv = mosqpp::mosquittopp::subscribe(NULL, m_topic.toLatin1().constData());
    if (MOSQ_ERR_SUCCESS != rv) {
        Q_WARN(QString("subscribe to '%1' failed: %2")
               .arg(m_topic)
               .arg(rv));
    }
}//MqClient::doSubscribe

void
MqClient::doDisconnect(void)
{
    Q_DEBUG("Disconnecting");
    mosqpp::mosquittopp::disconnect();
}//MqClient::doDisconnect

void
MqClient::doWorkLoop(void)
{
    int rv, count;

    count = m_eventsInLoop = 0;
    do {
        rv = this->loop(0);

        if (MOSQ_ERR_SUCCESS != rv) {
            if (MOSQ_ERR_CONN_LOST == rv) {
                Q_WARN("Lost connection");
                emit connectFailure();
            } else {
                Q_WARN(QString("loop returned failure: %1").arg(rv));
                stopWork();
            }
            break;
        }

        count++;
    } while ((0 != m_eventsInLoop) && (count < MQ_MAX_LOOPS));
}//MqClient::doWorkLoop
