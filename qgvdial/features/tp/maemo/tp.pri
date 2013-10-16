INCLUDEPATH += $$PWD

maemo5 {
    message(Maemo or Meego TP)
    INCLUDEPATH += $$QMAKESPEC/usr/include/telepathy-1.0/
    DEFINES += TP10
} else {
    error(Not maemo5 or harmattan!)
}

LIBS += -ltelepathy-qt4

HEADERS  += $$PWD/TpHeaders.h               \
            $$PWD/TpObserver.h              \
            $$PWD/TpCalloutInitiator.h      \
            $$PWD/TpAccountUtility.h
SOURCES  += $$PWD/TpObserver.cpp            \
            $$PWD/TpCalloutInitiator.cpp    \
            $$PWD/TpAccountUtility.cpp
