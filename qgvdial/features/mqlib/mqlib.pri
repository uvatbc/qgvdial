QT *= network statemachine

MQLIB = $$PWD/../../../third-party/mosquitto/lib
INCLUDEPATH	 += $$MQLIB $$MQLIB/cpp $$MQLIB/.. $$MQLIB/../include $$MQLIB/../deps

HEADERS  += $$MQLIB/alias_mosq.h \
            $$MQLIB/logging_mosq.h \
            $$MQLIB/memory_mosq.h \
            $$MQLIB/messages_mosq.h \
            $$MQLIB/misc_mosq.h \
            $$MQLIB/mosquitto_internal.h \
            $$MQLIB/net_mosq.h \
            $$MQLIB/packet_mosq.h \
            $$MQLIB/property_mosq.h \
            $$MQLIB/pthread_compat.h \
            $$MQLIB/read_handle.h \
            $$MQLIB/send_mosq.h \
            $$MQLIB/socks_mosq.h \
            $$MQLIB/time_mosq.h \
            $$MQLIB/tls_mosq.h \
            $$MQLIB/util_mosq.h \
            $$MQLIB/will_mosq.h

SOURCES  += $$MQLIB/actions.c \
            $$MQLIB/alias_mosq.c \
            $$MQLIB/callbacks.c \
            $$MQLIB/connect.c \
            $$MQLIB/handle_auth.c \
            $$MQLIB/handle_connack.c \
            $$MQLIB/handle_disconnect.c \
            $$MQLIB/handle_ping.c \
            $$MQLIB/handle_pubackcomp.c \
            $$MQLIB/handle_publish.c \
            $$MQLIB/handle_pubrec.c \
            $$MQLIB/handle_pubrel.c \
            $$MQLIB/handle_suback.c \
            $$MQLIB/handle_unsuback.c \
            $$MQLIB/helpers.c \
            $$MQLIB/logging_mosq.c \
            $$MQLIB/loop.c \
            $$MQLIB/memory_mosq.c \
            $$MQLIB/messages_mosq.c \
            $$MQLIB/misc_mosq.c \
            $$MQLIB/mosquitto.c \
            $$MQLIB/net_mosq.c \
            $$MQLIB/net_mosq_ocsp.c \
            $$MQLIB/options.c \
            $$MQLIB/packet_datatypes.c \
            $$MQLIB/packet_mosq.c \
            $$MQLIB/property_mosq.c \
            $$MQLIB/read_handle.c \
            $$MQLIB/send_connect.c \
            $$MQLIB/send_disconnect.c \
            $$MQLIB/send_mosq.c \
            $$MQLIB/send_publish.c \
            $$MQLIB/send_subscribe.c \
            $$MQLIB/send_unsubscribe.c \
            $$MQLIB/socks_mosq.c \
            $$MQLIB/srv_mosq.c \
            $$MQLIB/strings_mosq.c \
            $$MQLIB/thread_mosq.c \
            $$MQLIB/time_mosq.c \
            $$MQLIB/tls_mosq.c \
            $$MQLIB/utf8_mosq.c \
            $$MQLIB/util_mosq.c \
            $$MQLIB/util_topic.c \
            $$MQLIB/will_mosq.c

HEADERS  += $$MQLIB/cpp/mosquittopp.h
SOURCES  += $$MQLIB/cpp/mosquittopp.cpp

win32 {
    LIBS *= -lssleay32 -lWs2_32
} else {
    # Win32 doesn't have pthreads, everyone else does.
    DEFINES *= WITH_THREADING
}

DEFINES *= WITH_TLS WITH_TLS_PSK WITH_STATIC_MOSQ
# These aren't strictly required. They're here for completeness
DEFINES *= WITH_BRIDGE WITH_PERSISTENCE WITH_MEMORY_TRACKING WITH_SYS_TREE

win32 {
    LIBS *= -llibeay32
}
