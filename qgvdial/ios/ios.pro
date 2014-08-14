TEMPLATE = app
TARGET = qgvdial

MOC_DIR = moc
OBJECTS_DIR = obj

QT += qml quick widgets multimedia
QTPLUGIN += qsvg qjpeg

SOURCES  += MainWindow.cpp \
            BB10PhoneFactory.cpp \
            BBPhoneAccount.cpp

HEADERS  += platform_specific.h \
            MainWindow.h \
            BB10PhoneFactory.h \
            BBPhoneAccount.h

RESOURCES += qml.qrc \
             bb10.qrc

OTHER_FILES += \
    bar-descriptor.xml

# Additional import path used to resolve QML modules in Qt Creators code model
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
