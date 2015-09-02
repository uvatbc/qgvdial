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

MqClient::MqClient(const char *id, bool clean_session)
: mosquittopp(id, clean_session)
{
}//MqClient::MqClient

MqClient::~MqClient()
{
}//MqClient::~MqClient

void
MqClient::on_connect(int rc)
{
    emit connect(rc);
}//MqClient::on_connect

void
MqClient::on_disconnect(int rc)
{
    emit disconnect(rc);
}//MqClient::on_disconnect

void
MqClient::on_publish(int mid)
{
    emit publish(mid);
}//MqClient::on_publish

void
MqClient::on_message(const struct mosquitto_message *msg)
{
    emit message(msg);
}//MqClient::on_message

void
MqClient::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
    emit subscribe(mid, qos_count, granted_qos);
}//MqClient::on_subscribe

void
MqClient::on_unsubscribe(int mid)
{
    emit unsubscribe(mid);
}//MqClient::on_unsubscribe

void
MqClient::on_log(int level, const char *str)
{
    emit log(level, str);
}//MqClient::on_log

void
MqClient::on_error()
{
    emit error();
}//MqClient::on_error
