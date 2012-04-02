QT *= network
INCLUDEPATH	 += $$PWD

HEADERS  += $$PWD/logging_mosq.h        \
            $$PWD/memory_mosq.h         \
            $$PWD/messages_mosq.h       \
            $$PWD/mosquitto.h           \
            $$PWD/mosquitto_internal.h  \
            $$PWD/mqtt3_protocol.h      \
            $$PWD/net_mosq.h            \
            $$PWD/read_handle.h         \
            $$PWD/send_mosq.h           \
            $$PWD/util_mosq.h           \
            $$PWD/mq_config.h           \
            $$PWD/will_mosq.h

SOURCES  += $$PWD/logging_mosq.c        \
            $$PWD/memory_mosq.c         \
            $$PWD/messages_mosq.c       \
            $$PWD/mosquitto.c           \
            $$PWD/net_mosq.c            \
            $$PWD/read_handle.c         \
            $$PWD/read_handle_client.c  \
            $$PWD/read_handle_shared.c  \
            $$PWD/send_client_mosq.c    \
            $$PWD/send_mosq.c           \
            $$PWD/util_mosq.c           \
            $$PWD/will_mosq.c
