INCLUDEPATH += $$PWD
QT *= dbus

SOURCES  += $$PWD/SkypeClient.cpp \
            $$PWD/SkypeClientFactory.cpp \
            $$PWD/SkypeObserver.cpp \
            $$PWD/SkypeAccount.cpp
HEADERS  += $$PWD/SkypeClient.h \
            $$PWD/SkypeClientFactory.h \
            $$PWD/SkypeObserver.h \
            $$PWD/SkypeAccount.h

win32 {
SOURCES  += $$PWD/SkypeWinClient.cpp
HEADERS  += $$PWD/SkypeWinClient.h
} else {
SOURCES  += $$PWD/SkypeLinuxClient.cpp
HEADERS  += $$PWD/SkypeLinuxClient.h
}
