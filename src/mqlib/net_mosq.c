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
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <netinet\in.h>

#include <memory_mosq.h>
#include <net_mosq.h>

extern char *errStr;
extern int iErr;

void _mosquitto_net_init(void)
{
#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

#ifdef WITH_SSL
    SSL_library_init();
    OpenSSL_add_all_algorithms();
#endif
}

void _mosquitto_net_cleanup(void)
{
#ifdef WIN32
    WSACleanup();
#endif
}

void _mosquitto_packet_cleanup(struct _mosquitto_packet *packet)
{
    if(!packet) return;

    /* Free data and reset values */
    packet->command = 0;
    packet->have_remaining = 0;
    packet->remaining_count = 0;
    packet->remaining_mult = 1;
    packet->remaining_length = 0;
    if(packet->payload) _mosquitto_free(packet->payload);
    packet->payload = NULL;
    packet->to_process = 0;
    packet->pos = 0;
}

void _mosquitto_packet_queue(struct _mosquitto_core *core, struct _mosquitto_packet *packet)
{
    struct _mosquitto_packet *tail;

    assert(core);
    assert(packet);

    packet->pos = 0;
    packet->to_process = packet->packet_length;

    packet->next = NULL;
    if(core->out_packet){
        tail = core->out_packet;
        while(tail->next){
            tail = tail->next;
        }
        tail->next = packet;
    }else{
        core->out_packet = packet;
    }
}

/* Close a socket associated with a context and set it to -1.
 * Returns 1 on failure (context is NULL)
 * Returns 0 on success.
 */
int _mosquitto_socket_close(struct _mosquitto_core *core)
{
    int rc = 0;

    assert(core);
    /* FIXME - need to shutdown SSL here. */
    if(core->sock != INVALID_SOCKET){
        rc = COMPAT_CLOSE(core->sock);
        core->sock = INVALID_SOCKET;
    }

    return rc;
}

/* Create a socket and connect it to 'ip' on port 'port'.
 * Returns -1 on failure (ip is NULL, socket creation/connection error)
 * Returns sock number on success.
 */
int _mosquitto_socket_connect(struct _mosquitto_core *core, const char *host, uint16_t port)
{
    int sock = INVALID_SOCKET;
    int opt;
    struct addrinfo hints;
    struct addrinfo *ainfo, *rp;
    int s;
#ifdef WIN32
    uint32_t val = 1;
#endif
#ifdef WITH_SSL
    int ret;
#endif

    errStr = "host or port is NULL";
    if(!core || !host || !port) return MOSQ_ERR_INVAL;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_ADDRCONFIG;
    hints.ai_socktype = SOCK_STREAM;

    s = getaddrinfo(host, NULL, &hints, &ainfo);
    errStr = "getaddrinfo failed";
    if(s) return MOSQ_ERR_UNKNOWN;

    for(rp = ainfo; rp != NULL; rp = rp->ai_next){
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sock == INVALID_SOCKET) continue;

        if(rp->ai_family == PF_INET){
            ((struct sockaddr_in *)rp->ai_addr)->sin_port = htons(port);
        }else if(rp->ai_family == PF_INET6){
            ((struct sockaddr_in6 *)rp->ai_addr)->sin6_port = htons(port);
        }else{
            continue;
        }
        if(connect(sock, rp->ai_addr, rp->ai_addrlen) != -1){
            break;
        }

        iErr = errno;
        errStr = "connect failed";

        COMPAT_CLOSE(sock);
    }
    if(!rp){
        fprintf(stderr, "Error: %s\n", strerror(errno));
        errStr = strerror(errno);
        COMPAT_CLOSE(sock);
        return MOSQ_ERR_UNKNOWN;
    }
    freeaddrinfo(ainfo);

#ifdef WITH_SSL
    if(core->ssl){
        core->ssl->bio = BIO_new_socket(sock, BIO_NOCLOSE);
        if(!core->ssl->bio){
            COMPAT_CLOSE(sock);
            return MOSQ_ERR_SSL;
        }
        SSL_set_bio(core->ssl->ssl, core->ssl->bio, core->ssl->bio);

        ret = SSL_connect(core->ssl->ssl);
        if(ret != 1){
            COMPAT_CLOSE(sock);
            return MOSQ_ERR_SSL;
        }
    }
#endif

    /* Set non-blocking */
#ifndef WIN32
    opt = fcntl(sock, F_GETFL, 0);
    if(opt == -1 || fcntl(sock, F_SETFL, opt | O_NONBLOCK) == -1){
#ifdef WITH_SSL
        if(core->ssl){
            _mosquitto_free(core->ssl);
            core->ssl = NULL;
        }
#endif
        COMPAT_CLOSE(sock);
        errStr = "fcntl failed";
        return MOSQ_ERR_UNKNOWN;
    }
#else
    if(ioctlsocket(sock, FIONBIO, &val)){
#ifdef WITH_SSL
        if(core->ssl){
            _mosquitto_free(core->ssl);
            core->ssl = NULL;
        }
#endif
        COMPAT_CLOSE(sock);
        errStr = "ioctlsocket failed";
        return MOSQ_ERR_UNKNOWN;
    }
#endif

    core->sock = sock;
    errStr = NULL;
    return MOSQ_ERR_SUCCESS;
}

int _mosquitto_read_byte(struct _mosquitto_packet *packet, uint8_t *byte)
{
    assert(packet);
    if(packet->pos+1 > packet->remaining_length) return MOSQ_ERR_PROTOCOL;

    *byte = packet->payload[packet->pos];
    packet->pos++;

    return MOSQ_ERR_SUCCESS;
}

void _mosquitto_write_byte(struct _mosquitto_packet *packet, uint8_t byte)
{
    assert(packet);
    assert(packet->pos+1 <= packet->packet_length);

    packet->payload[packet->pos] = byte;
    packet->pos++;
}

int _mosquitto_read_bytes(struct _mosquitto_packet *packet, uint8_t *bytes, uint32_t count)
{
    assert(packet);
    if(packet->pos+count > packet->remaining_length) return MOSQ_ERR_PROTOCOL;

    memcpy(bytes, &(packet->payload[packet->pos]), count);
    packet->pos += count;

    return MOSQ_ERR_SUCCESS;
}

void _mosquitto_write_bytes(struct _mosquitto_packet *packet, const uint8_t *bytes, uint32_t count)
{
    assert(packet);
    assert(packet->pos+count <= packet->packet_length);

    memcpy(&(packet->payload[packet->pos]), bytes, count);
    packet->pos += count;
}

int _mosquitto_read_string(struct _mosquitto_packet *packet, char **str)
{
    uint16_t len;
    int rc;

    assert(packet);
    rc = _mosquitto_read_uint16(packet, &len);
    if(rc) return rc;

    if(packet->pos+len > packet->remaining_length) return MOSQ_ERR_PROTOCOL;

    *str = _mosquitto_calloc(len+1, sizeof(char));
    if(*str){
        memcpy(*str, &(packet->payload[packet->pos]), len);
        packet->pos += len;
    }else{
        return MOSQ_ERR_NOMEM;
    }

    return MOSQ_ERR_SUCCESS;
}

void _mosquitto_write_string(struct _mosquitto_packet *packet, const char *str, uint16_t length)
{
    assert(packet);
    _mosquitto_write_uint16(packet, length);
    _mosquitto_write_bytes(packet, (uint8_t *)str, length);
}

int _mosquitto_read_uint16(struct _mosquitto_packet *packet, uint16_t *word)
{
    uint8_t msb, lsb;

    assert(packet);
    if(packet->pos+2 > packet->remaining_length) return MOSQ_ERR_PROTOCOL;

    msb = packet->payload[packet->pos];
    packet->pos++;
    lsb = packet->payload[packet->pos];
    packet->pos++;

    *word = (msb<<8) + lsb;

    return MOSQ_ERR_SUCCESS;
}

void _mosquitto_write_uint16(struct _mosquitto_packet *packet, uint16_t word)
{
    _mosquitto_write_byte(packet, MOSQ_MSB(word));
    _mosquitto_write_byte(packet, MOSQ_LSB(word));
}

ssize_t _mosquitto_net_read(struct _mosquitto_core *core, void *buf, size_t count)
{
#ifdef WITH_SSL
    int ret;
    int err;
#endif
    assert(core);
#ifdef WITH_SSL
    if(core->ssl){
        ret = SSL_read(core->ssl->ssl, buf, count);
        if(ret < 0){
            err = SSL_get_error(core->ssl->ssl, ret);
            if(err == SSL_ERROR_WANT_READ){
                ret = -1;
                core->ssl->want_read = true;
                errno = EAGAIN;
            }else if(err == SSL_ERROR_WANT_WRITE){
                ret = -1;
                core->ssl->want_write = true;
                errno = EAGAIN;
            }
        }
        return (ssize_t )ret;
    }else{
        /* Call normal read/recv */

#endif

#ifndef WIN32
        return read(core->sock, buf, count);
#else
        return recv(core->sock, buf, count, 0);
#endif

#ifdef WITH_SSL
    }
#endif
}

ssize_t _mosquitto_net_write(struct _mosquitto_core *core, void *buf, size_t count)
{
#ifdef WITH_SSL
    int ret;
    int err;
#endif
    assert(core);

#ifdef WITH_SSL
    if(core->ssl){
        ret = SSL_write(core->ssl->ssl, buf, count);
        if(ret < 0){
            err = SSL_get_error(core->ssl->ssl, ret);
            if(err == SSL_ERROR_WANT_READ){
                ret = -1;
                core->ssl->want_read = true;
            }else if(err == SSL_ERROR_WANT_WRITE){
                ret = -1;
                core->ssl->want_write = true;
            }
        }
        return (ssize_t )ret;
    }else{
        /* Call normal write/send */
#endif

#ifndef WIN32
        return write(core->sock, buf, count);
#else
        return send(core->sock, buf, count, 0);
#endif

#ifdef WITH_SSL
    }
#endif
}

