TEMPLATE = app
TARGET = qgvdial

MOC_DIR = moc
OBJECTS_DIR = obj

QT += qml quick widgets multimedia

SOURCES  += MainWindow.cpp \
            IosPhoneFactory.cpp \
            IosPhoneAccount.cpp

HEADERS  += platform_specific.h \
            MainWindow.h \
            IosPhoneFactory.h \
            IosPhoneAccount.h

RESOURCES += qml.qrc \
             ios.qrc

# Additional import path used to resolve QML modules in Qt Creators code model
QML_IMPORT_PATH = qml

include(../common/common.pri)
include(../features/openssl/ios/ios-cipher.pri)
include(../features/dirs/ios/ios-dirs.pri)
include(../features/cookie-cutter/osdependent/cc-osdependent.pri)
include(../features/osver/bb10/osv-bb10.pri)
# include(../features/mqlib/mqlib.pri)
include(../features/qml/mainwindow/qml-mainwindow.pri)
include(../features/qml/qml5/qmlviewer.pri)

# Default rules for deployment.
include(deployment.pri)
