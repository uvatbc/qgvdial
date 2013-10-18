INCLUDEPATH += $$PWD

!exists($$QMAKESPEC/usr/include/telepathy-qt4/TelepathyQt/Constants) {
    error(libtelepathy-qt4 not installed!)
}

INCLUDEPATH += $$QMAKESPEC/usr/include/telepathy-qt4/
include(../common/tp-common.pri)

HEADERS  += $$PWD/TpHeaders.h

#HEADERS  += $$PWD/TpObserver.h
#SOURCES  += $$PWD/TpObserver.cpp

#HEADERS  += $$PWD/TpAccountUtility.h
#SOURCES  += $$PWD/TpAccountUtility.cpp
