include(common-code.pri)
include(../features/dirs/bb10/bb10-dirs.pri)

INCLUDEPATH += bb10
LIBS += -lbbsystem

MOC_DIR = moc
OBJECTS_DIR = obj

SOURCES  += bb10/BB10PhoneFactory.cpp \
            bb10/BBPhoneAccount.cpp

HEADERS  += bb10/platform_specific.h \
            bb10/BB10PhoneFactory.h \
            bb10/BBPhoneAccount.h
