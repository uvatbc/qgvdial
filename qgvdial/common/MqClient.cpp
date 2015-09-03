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

#include "MqClient.h"

#define MQ_WORK_UP_TIME     (2 * 1000)
#define MQ_WORK_DOWN_TIME   (3 * 1000)
#define MQ_LIMBO_TIME       (5 * 1000)

MqClient::MqClient(QObject *parent, const char *id, bool clean_session)
: QObject(parent)
, mosquittopp(id, clean_session)
, m_thread(NULL)
, m_sm(NULL)
, m_workTimer(NULL)
, m_workUptimePeriod(MQ_WORK_UP_TIME)
, m_workDowntimePeriod(MQ_WORK_DOWN_TIME)
, m_workLimboPeriod(MQ_LIMBO_TIME)
{
}//MqClient::MqClient

MqClient::~MqClient()
{
}//MqClient::~MqClient

void
MqClient::setupClient(QThread *thread,
                      const QString &host,
                      const int port,
                      const int keepalive)
{
    m_thread = thread;
    m_host = host;
    m_port = port;
    m_keepalive = keepalive;
}//MqClient::setupClient

void
MqClient::on_connect(int rc)
{
    emit sigConnect(rc);

    if (MOSQ_ERR_SUCCESS == rc) {
        Q_DEBUG("Connected");
    } else {
        Q_WARN("Connection failure");
        emit connectFailure();
    }
}//MqClient::on_connect

void
MqClient::on_disconnect(int rc)
{
    emit sigDisconnect(rc);
}//MqClient::on_disconnect

void
MqClient::on_publish(int mid)
{
    emit sigPublish(mid);
}//MqClient::on_publish

void
MqClient::on_message(const struct mosquitto_message *msg)
{
    emit sigMessage(msg);
}//MqClient::on_message

void
MqClient::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
    emit sigSubscribe(mid, qos_count, granted_qos);
}//MqClient::on_subscribe

void
MqClient::on_unsubscribe(int mid)
{
    emit sigUnsubscribe(mid);
}//MqClient::on_unsubscribe

void
MqClient::on_log(int level, const char *str)
{
    emit sigLog(level, str);
}//MqClient::on_log

void
MqClient::on_error()
{
    emit sigError();
}//MqClient::on_error

void
MqClient::startWork()
{
    if (recreateSm()) {
        Q_DEBUG("State machine started");
    }
    else {
        Q_WARN("Failed to create SM");
    }
}//MqClient::startWork

void
MqClient::stopWork(bool wait)
{
    emit sigStopWork();

    if (m_workTimer) {
        m_workTimer->stop();
        m_workTimer->deleteLater();
        m_workTimer = NULL;
    }

    if (m_thread) {
        m_thread->quit();

        if (wait) {
            m_thread->wait();
        }
    }
}//MqClient::stopWork

bool
MqClient::recreateTimer(void)
{
    if (m_workTimer) {
        m_workTimer->deleteLater();
    }

    m_workTimer = new QTimer(this);
    if (NULL == m_workTimer) {
        Q_WARN("Failed to allocate work timer");
        return false;
    }

    m_workTimer->setInterval(getDowntimePeriod());
    m_workTimer->setSingleShot(true);

    return true;
}//MqClient::recreateTimer

bool
MqClient::recreateSm(void)
{
    if (m_sm) {
        delete m_sm;
        m_sm = NULL;
    }

    if (!recreateTimer()) {
        return false;
    }

    m_sm = new QStateMachine(this);

    QState *initialState = new QState;
    QState *connectingState = new QState;
    QState *limboState = new QState;
    QState *loopState = new QState;
    QFinalState *endState = new QFinalState;

    if ((NULL == initialState) ||
        (NULL == connectingState) ||
        (NULL == limboState) ||
        (NULL == loopState) ||
        (NULL == endState))
    {
        Q_WARN("Some states could not be created!!");
        return false;
    }

    // Begin at init
    QObject::connect(initialState, SIGNAL(entered()),
                     this, SLOT(reinitMq()));
    // Work done in connecting state
    QObject::connect(connectingState, SIGNAL(entered()),
                     this, SLOT(reinitConnection()));
    // Work done in loop state
    QObject::connect(loopState, SIGNAL(entered()),
                     this, SLOT(doWorkLoop()));
    // Work done in limbo state
    QObject::connect(limboState, SIGNAL(entered()),
                     this, SLOT(doLimbo()));

    // beginConnect: init -> connecting
    initialState->addTransition(this, SIGNAL(beginConnect()), connectingState);
    // connecting -> loop
    connectingState->addTransition(this, SIGNAL(connectSuccess()), loopState);
    // connecting -> limbo
    connectingState->addTransition(this, SIGNAL(connectFailure()), limboState);
    // limbo -> connecting
    limboState->addTransition(this, SIGNAL(beginConnect()), connectingState);
    // loop -> loop for work
    loopState->addTransition(m_workTimer, SIGNAL(timeout()), loopState);
    // loop -> quit
    loopState->addTransition(this, SIGNAL(sigStopWork()), endState);
    // loop -> limbo
    loopState->addTransition(this, SIGNAL(connectFailure()), limboState);

    m_sm->addState(initialState);
    m_sm->addState(connectingState);
    m_sm->addState(limboState);
    m_sm->addState(loopState);
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
MqClient::reinitConnection(void)
{
    int rv;
    rv = mosqpp::mosquittopp::connect(m_host.toLatin1().constData(),
                                      m_port,
                                      m_keepalive);
    if (MOSQ_ERR_SUCCESS == rv) {
        Q_DEBUG("Connection attempt started");
        emit connectSuccess();
        return;
    }

    Q_WARN(QString("Connection attempt failed. rv = %1").arg(rv));
    emit connectFailure();
}//MqClient::reinitConnection

void
MqClient::doLimbo(void)
{
    QTimer::singleShot(getLimboPeriod(), this, SIGNAL(beginConnect()));
}//MqClient::doLimbo

void
MqClient::doWorkLoop(void)
{
    int rv = this->loop(getUptimePeriod());

    if (MOSQ_ERR_SUCCESS != rv) {
        if (MOSQ_ERR_CONN_LOST == rv) {
            Q_WARN("Lost connection");
            emit connectFailure();
            return;
        }

        Q_WARN(QString("loop returned failure: %1").arg(rv));
        stopWork(false);
        return;
    }

    Q_DEBUG("Doing something!");

    if (m_workTimer) {
        m_workTimer->start();
    }
}//MqClient::doWorkLoop
