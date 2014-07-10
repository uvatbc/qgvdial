TEMPLATE = app
TARGET = qgvdial

MOC_DIR = moc
OBJECTS_DIR = obj

QT += qml quick widgets

SOURCES  += BB10PhoneFactory.cpp \
            MainWindow.cpp
HEADERS  += platform_specific.h \
            BB10PhoneFactory.h \
            MainWindow.h

# So that I can open this project in the Qt SDK and have it compile correctly:
!simulator {
SOURCES  += BBPhoneAccount.cpp
HEADERS  += BBPhoneAccount.h
LIBS += -lbbsystem
}

RESOURCES += qml.qrc \
             bb10.qrc

OTHER_FILES += \
    bar-descriptor.xml

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = qml

include(../common/common.pri)
include(../features/openssl/openssl-lin.pri)
include(../features/dirs/bb10/bb10-dirs.pri)
include(../features/cookie-cutter/osdependent/cc-osdependent.pri)
include(../features/osver/bb10/osv-bb10.pri)
include(../features/mqlib/mqlib.pri)
include(../features/qml/mainwindow/qml-mainwindow.pri)
include(../features/qml/qml5/qmlviewer.pri)

# Default rules for deployment.
include(deployment.pri)
