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
# Installation related lines go here
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
    INSTALLS += target
    target.path = $$OPTPREFIX/qgvdial/bin
}

# Installation for Linux
unix:!symbian:!maemo5:!contains(DEFINES,MEEGO_HARMATTAN) {
    DATADIR = $$PREFIX/share
    INSTALLS += target
    target.path = $$PREFIX/bin
}

