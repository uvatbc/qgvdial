TEMPLATE = app
TARGET = qgv-tp
DEPENDPATH += .
INCLUDEPATH += .

QT += dbus

MOC_DIR = moc
OBJECTS_DIR = obj

# Input
HEADERS += global.h                     \
           shared_data_types.h          \
           gen/cm_adapter.h             \
           gen/connection_adapter.h     \
           gen/protocol_adapter.h       \
           QGVConnectionManager.h       \
           QGVConnection.h

SOURCES += shared_data_types.cpp        \
           gen/cm_adapter.cpp           \
           gen/connection_adapter.cpp   \
           gen/protocol_adapter.cpp     \
           main.cpp                     \
           QGVConnectionManager.cpp     \
           QGVConnection.cpp

exists(not-really) {
HEADERS += gen/cm_proxy.h               \
           gen/connection_proxy.h       \
           gen/protocol_proxy.h

SOURCES += gen/cm_proxy.cpp             \
           gen/connection_proxy.cpp     \
           protocol_proxy.cpp
}

OTHER_FILES +=              \
    protocol.xml            \
    connection.xml          \
    cm.xml                  \
    connection-ifaces.xml   \
    channel.xml
