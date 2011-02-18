QT       += dbus core
QT       -= gui

TARGET = qgv-tp
CONFIG   += qt debug console
CONFIG   -= app_bundle

TEMPLATE = app
VPATH += src
#MOC_DIR = mocs
#OBJECTS_DIR = objs

SOURCES  += src/main.cpp \
            src/connectiontypes.cpp \
            src/connectionmanagertypes.cpp \
            src/connectionmanageradaptor.cpp \
            src/connectionmanager.cpp \
            src/connectionadaptor.cpp \
            src/connection.cpp \
            src/connectioninterfacerequeststypes.cpp \
            src/connectioninterfacerequestsadaptor.cpp \
            src/connectioninterfacecapabilitiestypes.cpp \
            src/connectioninterfacecapabilitiesadaptor.cpp

HEADERS  += src/names.h \
            src/connectiontypes.h \
            src/connectionmanagertypes.h \
            src/connectionmanageradaptor.h \
            src/connectionmanager.h \
            src/connectionadaptor.h \
            src/connection.h \
            src/basetypes.h \
            src/connectioninterfacerequeststypes.h \
            src/connectioninterfacerequestsadaptor.h \
            src/connectioninterfacecapabilitiestypes.h \
            src/connectioninterfacecapabilitiesadaptor.h

###############################################################
# Installation related line go here
###############################################################

# Installation for maemo
maemo5 {
    exists(../../buildit.sh) {
        PREFIX = ../debian/qgvdial/usr
        message(Built using my scripts... probably inside scratchbox)
    }
    exists(../../buildit.pl) {
        PREFIX = ../debian/qgvdial/usr
        message(Built using my scripts)
    }
    !exists(../../buildit.pl):!exists(../../buildit.sh) {
        PREFIX = ../maemo/debian/qgvdial/usr
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

