/*
Copyright (c) 2009-2011 Roger Light <roger@atchoo.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of mosquitto nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <assert.h>
#include <string.h>

#include <mosquitto.h>
#include <memory_mosq.h>
#include <mqtt3_protocol.h>
#include <net_mosq.h>
#include <send_mosq.h>
#include <util_mosq.h>

int _mosquitto_send_connect(struct mosquitto *mosq, uint16_t keepalive, bool clean_session)
{
	struct _mosquitto_packet *packet = NULL;
	int payloadlen;
	uint8_t will = 0;
	uint8_t byte;
	int rc;

	assert(mosq);
	assert(mosq->core.id);

	packet = _mosquitto_calloc(1, sizeof(struct _mosquitto_packet));
	if(!packet) return MOSQ_ERR_NOMEM;

	payloadlen = 2+strlen(mosq->core.id);
	if(mosq->core.will){
		will = 1;
		assert(mosq->core.will->topic);

		payloadlen += 2+strlen(mosq->core.will->topic) + 2+mosq->core.will->payloadlen;
	}
	if(mosq->core.username){
		payloadlen += 2+strlen(mosq->core.username);
		if(mosq->core.password){
			payloadlen += 2+strlen(mosq->core.password);
		}
	}

	packet->command = CONNECT;
	packet->remaining_length = 12+payloadlen;
	rc = _mosquitto_packet_alloc(packet);
	if(rc){
		_mosquitto_free(packet);
		return rc;
	}

	/* Variable header */
	_mosquitto_write_string(packet, PROTOCOL_NAME, strlen(PROTOCOL_NAME));
	_mosquitto_write_byte(packet, PROTOCOL_VERSION);
	byte = (clean_session&0x1)<<1;
	if(will){
		byte = byte | ((mosq->core.will->retain&0x1)<<5) | ((mosq->core.will->qos&0x3)<<3) | ((will&0x1)<<2);
	}
	if(mosq->core.username){
		byte = byte | 0x1<<7;
		if(mosq->core.password){
			byte = byte | 0x1<<6;
		}
	}
	_mosquitto_write_byte(packet, byte);
	_mosquitto_write_uint16(packet, keepalive);

	/* Payload */
	_mosquitto_write_string(packet, mosq->core.id, strlen(mosq->core.id));
	if(will){
		_mosquitto_write_string(packet, mosq->core.will->topic, strlen(mosq->core.will->topic));
		_mosquitto_write_string(packet, (const char *)mosq->core.will->payload, mosq->core.will->payloadlen);
	}
	if(mosq->core.username){
		_mosquitto_write_string(packet, mosq->core.username, strlen(mosq->core.username));
		if(mosq->core.password){
			_mosquitto_write_string(packet, mosq->core.password, strlen(mosq->core.password));
		}
	}

	mosq->core.keepalive = keepalive;
	_mosquitto_packet_queue(&mosq->core, packet);
	return MOSQ_ERR_SUCCESS;
}

int _mosquitto_send_disconnect(struct mosquitto *mosq)
{
	assert(mosq);
	return _mosquitto_send_simple_command(mosq, DISCONNECT);
}

int _mosquitto_send_subscribe(struct mosquitto *mosq, uint16_t *mid, bool dup, const char *topic, uint8_t topic_qos)
{
	/* FIXME - only deals with a single topic */
	struct _mosquitto_packet *packet = NULL;
	uint32_t packetlen;
	uint16_t local_mid;
	int rc;

	assert(mosq);
	assert(topic);

	packet = _mosquitto_calloc(1, sizeof(struct _mosquitto_packet));
	if(!packet) return MOSQ_ERR_NOMEM;

	packetlen = 2 + 2+strlen(topic) + 1;

	packet->command = SUBSCRIBE | (dup<<3) | (1<<1);
	packet->remaining_length = packetlen;
	rc = _mosquitto_packet_alloc(packet);
	if(rc){
		_mosquitto_free(packet);
		return rc;
	}

	/* Variable header */
	local_mid = _mosquitto_mid_generate(&mosq->core);
	if(mid) *mid = local_mid;
	_mosquitto_write_uint16(packet, local_mid);

	/* Payload */
	_mosquitto_write_string(packet, topic, strlen(topic));
	_mosquitto_write_byte(packet, topic_qos);

	_mosquitto_packet_queue(&mosq->core, packet);
	return MOSQ_ERR_SUCCESS;
}


int _mosquitto_send_unsubscribe(struct mosquitto *mosq, uint16_t *mid, bool dup, const char *topic)
{
	/* FIXME - only deals with a single topic */
	struct _mosquitto_packet *packet = NULL;
	uint32_t packetlen;
	uint16_t local_mid;
	int rc;

	assert(mosq);
	assert(topic);

	packet = _mosquitto_calloc(1, sizeof(struct _mosquitto_packet));
	if(!packet) return MOSQ_ERR_NOMEM;

	packetlen = 2 + 2+strlen(topic);

	packet->command = UNSUBSCRIBE | (dup<<3) | (1<<1);
	packet->remaining_length = packetlen;
	rc = _mosquitto_packet_alloc(packet);
	if(rc){
		_mosquitto_free(packet);
		return rc;
	}

	/* Variable header */
	local_mid = _mosquitto_mid_generate(&mosq->core);
	if(mid) *mid = local_mid;
	_mosquitto_write_uint16(packet, local_mid);

	/* Payload */
	_mosquitto_write_string(packet, topic, strlen(topic));

	_mosquitto_packet_queue(&mosq->core, packet);
	return MOSQ_ERR_SUCCESS;
}

