QT *= network

INCLUDEPATH	 += $$PWD $$PWD/cpp

HEADERS  += $$PWD/mosquitto.h           \
            $$PWD/logging_mosq.h        \
            $$PWD/memory_mosq.h         \
            $$PWD/messages_mosq.h       \
            $$PWD/net_mosq.h            \
            $$PWD/read_handle.h         \
            $$PWD/send_mosq.h           \
            $$PWD/time_mosq.h           \
            $$PWD/tls_mosq.h            \
            $$PWD/util_mosq.h           \
            $$PWD/will_mosq.h           \
            $$PWD/mosquitto_internal.h  \
            $$PWD/mqtt3_protocol.h      \
            $$PWD/mq_config.h

SOURCES  += $$PWD/mosquitto.c           \
            $$PWD/logging_mosq.c        \
            $$PWD/memory_mosq.c         \
            $$PWD/messages_mosq.c       \
            $$PWD/net_mosq.c            \
            $$PWD/read_handle.c         \
            $$PWD/read_handle_client.c  \
            $$PWD/read_handle_shared.c  \
            $$PWD/send_mosq.c           \
            $$PWD/send_client_mosq.c    \
            $$PWD/thread_mosq.c         \
            $$PWD/time_mosq.c           \
            $$PWD/tls_mosq.c            \
            $$PWD/util_mosq.c           \
            $$PWD/will_mosq.c           \
            $$PWD/srv_mosq.c

HEADERS  += $$PWD/cpp/mosquittopp.h
SOURCES  += $$PWD/cpp/mosquittopp.cpp

win32 {
    LIBS *= -lssleay32 -lWs2_32
} else {
!symbian {
    # Win32 and symbian don't have pthreads, everyone else does.
    DEFINES *= WITH_THREADING
}
}

DEFINES *= WITH_TLS WITH_TLS_PSK WITH_STATIC_MOSQ
# These aren't strictly required. They're here for completeness
DEFINES *= WITH_BRIDGE WITH_PERSISTENCE WITH_MEMORY_TRACKING WITH_SYS_TREE

blackberry {
    DEFINES *= NO_PSELECT
}

win32 {
    LIBS *= -llibeay32
}
