INCLUDEPATH += $$PWD
QT *= dbus

greaterThan(QT_MAJOR_VERSION, 4) {
LIBS += -ltelepathy-qt5
} else {
LIBS += -ltelepathy-qt4
}

HEADERS  += $$PWD/TpCalloutInitiator.h \
            $$PWD/TpPhoneFactory.h \
            $$PWD/TpTask.h
SOURCES  += $$PWD/TpCalloutInitiator.cpp \
            $$PWD/TpPhoneFactory.cpp \
            $$PWD/TpTask.cpp
