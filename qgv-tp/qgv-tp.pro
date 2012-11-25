QT       += dbus core
QT       -= gui

TARGET = qgv-tp
CONFIG   += qt debug console
CONFIG   -= app_bundle

TEMPLATE = app
VPATH += src
INCLUDEPATH += src

MOC_DIR = moc
OBJECTS_DIR = obj

# Input
HEADERS += src/global.h                     \
           src/shared_data_types.h          \
           src/gen_textchannel.h            \
           src/gen/channel_adapter.h        \
           src/gen/textchannel_adapter.h    \
           src/gen/cm_adapter.h             \
           src/gen/connection_adapter.h     \
           src/gen/protocol_adapter.h       \
           src/QGVConnectionManager.h       \
           src/QGVConnection.h              \
           src/QGVChannel.h                 \
           src/QGVTextChannel.h

SOURCES += src/shared_data_types.cpp        \
           src/gen/channel_adapter.cpp      \
           src/gen/textchannel_adapter.cpp  \
           src/gen/cm_adapter.cpp           \
           src/gen/connection_adapter.cpp   \
           src/gen/protocol_adapter.cpp     \
           src/main.cpp                     \
           src/QGVConnectionManager.cpp     \
           src/QGVConnection.cpp            \
           src/QGVChannel.cpp               \
           src/QGVTextChannel.cpp

OTHER_FILES += src/protocol.xml             \
               src/connection.xml           \
               src/cm.xml                   \
               src/connection-ifaces.xml    \
               src/channel.xml              \
               src/textchannel.xml

###############################################################
# Installation related line go here
###############################################################

# Installation for maemo
maemo5 {
    exists(../../buildit.sh) {
        PREFIX = ../debian/qgvtp/usr
        message(Built using my scripts... probably inside scratchbox)
    }
    exists(../../buildit.pl) {
        PREFIX = ../debian/qgvtp/usr
        message(Built using my scripts)
    }
    !exists(../../buildit.pl):!exists(../../buildit.sh) {
        PREFIX = ../maemo/debian/qgvtp/usr
        message(Build using qtcreator)
    }

    message(maemo5 install)
    OPTPREFIX  = $$PREFIX/../opt/qgvdial
    BINDIR     = $$OPTPREFIX/bin
    DATADIR    = $$PREFIX/share
    OPTDATADIR = $$OPTPREFIX/share

    DEFINES += DATADIR=\"$$DATADIR\" PKGDATADIR=\"$$PKGDATADIR\"

    INSTALLS += target service manager qprofile

    target.path =$$BINDIR

    service.path = $$DATADIR/dbus-1/services
    service.files += ../data/org.freedesktop.Telepathy.ConnectionManager.qgvtp.service

    manager.path = $$DATADIR/telepathy/managers
    manager.files += ../data/qgvtp.manager

    qprofile.path = $$DATADIR/osso-rtcom
    qprofile.files += ../data/qgvtp.profile
}

# Installation for Linux
unix:!symbian:!maemo5 {
    BINDIR  = $$PREFIX/bin
    DATADIR = $$PREFIX/share
    message($$BINDIR)

    DEFINES += DATADIR=\"$$DATADIR\" PKGDATADIR=\"$$PKGDATADIR\"

    INSTALLS += target service manager

    target.path =$$BINDIR

    service.path = $$DATADIR/dbus-1/services
    service.files += ../data/org.freedesktop.Telepathy.ConnectionManager.qgvtp.service

    manager.path = $$DATADIR/telepathy/managers
    manager.files += ../data/qgvtp.manager
}

