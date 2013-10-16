INCLUDEPATH += $$PWD

QT *= dbus

!exists($$QMAKESPEC/usr/include/telepathy-qt4/TelepathyQt/Constants) {
    error(libtelepathy-qt4 not installed!)
}

INCLUDEPATH += $$QMAKESPEC/usr/include/telepathy-qt4/
LIBS += -ltelepathy-qt4

HEADERS  += $$PWD/TpHeaders.h

HEADERS  += $$PWD/TpCalloutInitiator.h
SOURCES  += $$PWD/TpCalloutInitiator.cpp

#HEADERS  += $$PWD/TpObserver.h
#SOURCES  += $$PWD/TpObserver.cpp

#HEADERS  += $$PWD/TpAccountUtility.h
#SOURCES  += $$PWD/TpAccountUtility.cpp
