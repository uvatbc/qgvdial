QT *= sql
QT -= gui

TARGET = bbphone
TEMPLATE = lib

DEFINES += QT4_LIBRARY

SOURCES  += BB10PhoneFunctions.cpp

HEADERS  += inc/bb10_qt4_global.h

INCLUDEPATH += inc
LIBS += -lbbsystem

MOC_DIR = moc
OBJECTS_DIR = obj

unix {
    target.path = /usr/lib
    INSTALLS += target
}

OTHER_FILES += \
    bar-descriptor.xml
