INCLUDEPATH += $$PWD
QT *= dbus
LIBS += -ltelepathy-qt4

HEADERS  += $$PWD/TpCalloutInitiator.h \
            $$PWD/TpPhoneFactory.h \
            $$PWD/TpTask.h
SOURCES  += $$PWD/TpCalloutInitiator.cpp \
            $$PWD/TpPhoneFactory.cpp \
            $$PWD/TpTask.cpp
