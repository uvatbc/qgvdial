INCLUDEPATH += $$PWD

QT *= dbus

HEADERS  += $$PWD/gen/api_adapter.h  \
            $$PWD/DBusApi.h
SOURCES  += $$PWD/gen/api_adapter.cpp \
            $$PWD/DBusApi.cpp
