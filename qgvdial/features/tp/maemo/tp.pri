INCLUDEPATH += $$PWD

QT *= dbus

message(Maemo or Meego TP)
INCLUDEPATH += $$QMAKESPEC/usr/include/telepathy-1.0/
DEFINES += TP10

LIBS += -ltelepathy-qt4

HEADERS  += $$PWD/TpHeaders.h

HEADERS  += $$PWD/TpCalloutInitiator.h
SOURCES  += $$PWD/TpCalloutInitiator.cpp

#HEADERS  += $$PWD/TpObserver.h
#SOURCES  += $$PWD/TpObserver.cpp

#HEADERS  += $$PWD/TpAccountUtility.h
#SOURCES  += $$PWD/TpAccountUtility.cpp
