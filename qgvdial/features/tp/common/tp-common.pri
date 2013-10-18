INCLUDEPATH += $$PWD
QT *= dbus
LIBS += -ltelepathy-qt4

HEADERS  += $$PWD/TpCalloutInitiator.h \
    ../features/tp/common/TpPhoneFactory.h
SOURCES  += $$PWD/TpCalloutInitiator.cpp \
    ../features/tp/common/TpPhoneFactory.cpp

