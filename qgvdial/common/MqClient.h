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
    MqClient(const char *id = NULL, bool clean_session = true);
    ~MqClient();

signals:
    void connect(int rc);
    void disconnect(int rc);
    void publish(int mid);
    void message(const struct mosquitto_message *message);
    void subscribe(int mid, int qos_count, const int *granted_qos);
    void unsubscribe(int mid);
    void log(int level, const char *str);
    void error();

protected:
    void on_connect(int rc);
    void on_disconnect(int rc);
    void on_publish(int mid);
    void on_message(const struct mosquitto_message *msg);
    void on_subscribe(int mid, int qos_count, const int *granted_qos);
    void on_unsubscribe(int mid);
    void on_log(int level, const char *str);
    void on_error();
};

#endif//_MQCLIENT_H_
