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

# For the beta version of the SDK, this part will identify Meego/Harmattan
exists($$QMAKE_INCDIR_QT"/../qmsystem2/qmkeys.h"):!contains(MEEGO_EDITION,harmattan): {
  MEEGO_VERSION_MAJOR     = 1
  MEEGO_VERSION_MINOR     = 2
  MEEGO_VERSION_PATCH     = 0
  MEEGO_EDITION           = harmattan
}

contains(MEEGO_EDITION,harmattan) {
  message(Meego!)
  DEFINES += MEEGO_HARMATTAN
}

###############################################################
# Installation related info goes here
###############################################################

exists(../isHarmattan) {
    DEBDIR = qgvdial
    message(Using qgvdial as the debian directory)
} else {
    DEBDIR = qgvtp
    message(Using qgvtp as the debian directory)
}

maemo5 {
    message(maemo5 install)
    exists(../../buildit.sh) || exists(../../buildit.pl) || exists(.svn) {
        PREFIX = ../debian/$$DEBDIR/usr
        message(Built using my scripts... probably inside scratchbox)
    } else {
        PREFIX = ../maemo/debian/$$DEBDIR/usr
        message(Build using qtcreator)
    }

    OPTPREFIX  = $$PREFIX/../opt
    DATADIR    = $$PREFIX/share
}

contains(DEFINES,MEEGO_HARMATTAN) {
    message(Harmattan install)

    OPTPREFIX  = /opt
    DATADIR    = /usr/share
}

# Installation for maemo
maemo5|contains(DEFINES,MEEGO_HARMATTAN) {
    INSTALLS += target service manager qprofile

    target.path = $$OPTPREFIX/qgvdial/bin

    service.path = $$DATADIR/dbus-1/services
    service.files += ../data/org.freedesktop.Telepathy.ConnectionManager.qgvtp.service

    manager.path = $$DATADIR/telepathy/managers
    manager.files += ../data/qgvtp.manager

    qprofile.path = $$DATADIR/osso-rtcom
    qprofile.files += ../data/qgvtp.profile
}

# Installation for Linux
unix:!symbian:!maemo5:!contains(DEFINES,MEEGO_HARMATTAN) {
    DATADIR = $$PREFIX/share
    message($$PREFIX/bin)

    INSTALLS += target service manager

    target.path = $$PREFIX/bin

    service.path = $$DATADIR/dbus-1/services
    service.files += ../data/org.freedesktop.Telepathy.ConnectionManager.qgvtp.service

    manager.path = $$DATADIR/telepathy/managers
    manager.files += ../data/qgvtp.manager
}

