INCLUDEPATH += $$PWD

QT *= dbus
maemo5 {
    message(Maemo or Meego TP)
    INCLUDEPATH += $$QMAKESPEC/usr/include/telepathy-1.0/
    DEFINES += TP10
} else {
    exists($$QMAKESPEC/usr/include/telepathy-qt4/TelepathyQt/Constants) {
        message(Brand new TP)
        INCLUDEPATH += $$QMAKESPEC/usr/include/telepathy-qt4/
    } else {
        message(Old TP)
        INCLUDEPATH += $$QMAKESPEC/usr/include/telepathy-1.0/
        DEFINES += TP10
    }
}

LIBS += -ltelepathy-qt4 -lssl -lcrypto

HEADERS  += $$PWD/TpObserver.h          \
            $$PWD/TpCalloutInitiator.h  \
            $$PWD/../gen/api_adapter.h  \
            $$PWD/DBusApi.h             \
            $$PWD/TpAccountUtility.h
SOURCES  += $$PWD/TpObserver.cpp        \
            $$PWD/TpCalloutInitiator.cpp \
            $$PWD/../gen/api_adapter.cpp \
            $$PWD/DBusApi.cpp           \
            $$PWD/TpAccountUtility.cpp
