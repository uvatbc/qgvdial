QT      *= core network sql xml xmlpatterns script
TARGET   = qgvnotify
TEMPLATE = app
INCLUDEPATH=../src

CONFIG  *= precompile_header mobility console

# In Windows, add the mosquitto dll
win32 {
    CONFIG *= embed_manifest_exe
    LIBS *= -lmosquitto -llibeay32
}

unix:!symbian {
    QT *= dbus
    LIBS *= -lmosquitto -lssl -lcrypto

    exists(../../buildit.sh) {
        PREFIX = ../debian/qgvnotify/usr
        message(Built using my scripts... probably inside scratchbox)
    } else {
        PREFIX = /opt/usr
    }

    OPTPREFIX  = $$PREFIX/../opt/qgvdial
    BINDIR     = $$OPTPREFIX/bin
    DATADIR    = $$PREFIX/share
    OPTDATADIR = $$OPTPREFIX/share
    DEFINES += DATADIR=\"$$DATADIR\" PKGDATADIR=\"$$PKGDATADIR\"

    target.path =$$BINDIR
    INSTALLS += target

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
}

PRECOMPILED_HEADER = ../src/global.h

SOURCES  += main.cpp                        \
            MainWindow.cpp                  \
            NotifyGVContactsTable.cpp       \
            NotifyGVInbox.cpp               \
            MqPublisher.cpp                 \
            ../src/GVApi.cpp                \
            ../src/AsyncTaskToken.cpp       \
            ../src/CookieJar.cpp            \
            ../src/NwReqTracker.cpp         \
            ../src/GvXMLParser.cpp          \
            ../src/ContactsParserObject.cpp \
            ../src/ContactsXmlHandler.cpp   \
            ../src/MyXmlErrorHandler.cpp


HEADERS  += ../src/global.h                 \
            MainWindow.h                    \
            NotifyGVContactsTable.h         \
            NotifyGVInbox.h                 \
            MqPublisher.h                   \
            ../src/GVApi.h                  \
            ../src/AsyncTaskToken.h         \
            ../src/CookieJar.h              \
            ../src/NwReqTracker.h           \
            ../src/GvXMLParser.h            \
            ../src/ContactsParserObject.h   \
            ../src/ContactsXmlHandler.h     \
            ../src/MyXmlErrorHandler.h
