INCLUDEPATH += $$PWD

message(Maemo or Meego TP)
INCLUDEPATH += $$QMAKESPEC/usr/include/telepathy-1.0/
DEFINES += TP10

include(../common/tp-common.pri)

HEADERS  += $$PWD/TpHeaders.h

#HEADERS  += $$PWD/TpObserver.h
#SOURCES  += $$PWD/TpObserver.cpp

#HEADERS  += $$PWD/TpAccountUtility.h
#SOURCES  += $$PWD/TpAccountUtility.cpp
