QT       += dbus core
QT       -= gui

TEMPLATE = app
TARGET = qgv-util
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS  += telepathyutility.h      \
            accountproxy.h          \
            accountmanagerproxy.h   \
            accountcompatproxy.h

SOURCES  += main.cpp                \
            telepathyutility.cpp    \
            accountproxy.cpp        \
            accountmanagerproxy.cpp \
            accountcompatproxy.cpp

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

    INSTALLS += target

    target.path =$$BINDIR
}

# Installation for Linux
unix:!symbian:!maemo5 {
    BINDIR  = $$PREFIX/bin
    DATADIR = $$PREFIX/share
    message($$BINDIR)

    DEFINES += DATADIR=\"$$DATADIR\" PKGDATADIR=\"$$PKGDATADIR\"

    INSTALLS += target

    target.path =$$BINDIR
}

