QT      *= core network sql xml xmlpatterns script
TARGET   = qgvnotify
TEMPLATE = app
INCLUDEPATH=../src

MOC_DIR = moc
OBJECTS_DIR = obj

CONFIG  *= precompile_header mobility console
DEFINES += DISABLE_TELEPATHY

# In Windows, add the mosquitto dll
win32 {
    CONFIG *= embed_manifest_exe
    LIBS *= -lmosquitto -llibeay32
}

unix:!symbian {
    QT *= dbus
    LIBS *= -lmosquitto -lssl -lcrypto

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
            ../src/MyXmlErrorHandler.cpp    \
            dbusinterface/qgvn_adapter.cpp  \
            dbusinterface/qgvn_proxy.cpp

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
            ../src/MyXmlErrorHandler.h      \
            dbusinterface/qgvn_adapter.h    \
            dbusinterface/qgvn_proxy.h

# Installation for Maemo and Harmattan
maemo5 {
    message(maemo5 install)
    exists(../../buildit.sh) || exists(../../buildit.pl) || exists(.svn) {
        PREFIX = ../debian/qgvnotify/usr
        message(Built using my scripts... probably inside scratchbox)
    } else {
        PREFIX = ../maemo/debian/qgvnotify/usr
        message(Build using qtcreator)
    }
    
    OPTPREFIX  = $$PREFIX/../opt
    DATADIR    = $$PREFIX/share
}

contains(MEEGO_EDITION,harmattan) {
    message(Harmattan install)
    
    OPTPREFIX  = /opt
    DATADIR    = /usr/share
}

maemo5|contains(MEEGO_EDITION,harmattan) {
    target.path = $$OPTPREFIX/qgvdial/bin
    INSTALLS += target
}
