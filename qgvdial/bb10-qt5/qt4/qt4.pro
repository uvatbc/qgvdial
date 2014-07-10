QT *= sql
QT -= gui

TARGET = bbphone
TEMPLATE = lib

DEFINES += QT4_LIBRARY

SOURCES  += BB10PhoneFactory.cpp \
            BBPhoneAccount.cpp \
            BBPhoneAccountPrivate.cpp

HEADERS  += BB10PhoneFactory.h \
            BBPhoneAccount.h \ \
            BBPhoneAccountPrivate.h \
            qt4_global.h \
            platform_specific.h

LIBS += -lbbsystem

MOC_DIR = moc
OBJECTS_DIR = obj

include(../../common/phone-account.pri)
include(../../../api/api.pri)

unix {
    target.path = /usr/lib
    INSTALLS += target
}

OTHER_FILES += \
    bar-descriptor.xml
