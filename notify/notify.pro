QT      *= core gui network webkit sql xml xmlpatterns script
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
    LIBS *= -lmosquitto -lssl

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
}

PRECOMPILED_HEADER = ../src/global.h

SOURCES  += main.cpp                        \
            MainWindow.cpp                  \
            NotifySingletons.cpp            \
            NotifyGVContactsTable.cpp       \
            NotifyGVInbox.cpp               \
            MqPublisher.cpp                 \
            ../src/GVAccess.cpp             \
            ../src/GVWebPage.cpp            \
            ../src/MobileWebPage.cpp        \
            ../src/GvXMLParser.cpp          \
            ../src/ContactsParserObject.cpp \
            ../src/ContactsXmlHandler.cpp


HEADERS  += ../src/global.h                 \
            MainWindow.h                    \
            NotifySingletons.h              \
            NotifyGVContactsTable.h         \
            NotifyGVInbox.h                 \
            MqPublisher.h                   \
            ../src/GVAccess.h               \
            ../src/GVWebPage.h              \
            ../src/MobileWebPage.h          \
            ../src/GvXMLParser.h            \
            ../src/ContactsParserObject.h   \
            ../src/ContactsXmlHandler.h
