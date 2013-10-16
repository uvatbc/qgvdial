INCLUDEPATH += $$PWD

!exists($$QMAKESPEC/usr/include/telepathy-qt4/TelepathyQt/Constants) {
    error(libtelepathy-qt4 not installed!)
}

INCLUDEPATH += $$QMAKESPEC/usr/include/telepathy-qt4/
LIBS += -ltelepathy-qt4

HEADERS  += $$PWD/TpHeaders.h               \
            $$PWD/TpObserver.h              \
            $$PWD/TpCalloutInitiator.h      \
            $$PWD/TpAccountUtility.h
SOURCES  += $$PWD/TpObserver.cpp            \
            $$PWD/TpCalloutInitiator.cpp    \
            $$PWD/TpAccountUtility.cpp
