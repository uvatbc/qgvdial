TEMPLATE = app
TARGET = qgvdial

QT += qml quick widgets multimedia

INCLUDEPATH *= $$PWD

SOURCES  += MainWindow.cpp \
            WP8PhoneFactory.cpp

HEADERS  += platform_specific.h \
            MainWindow.h \
            WP8PhoneFactory.h

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = qml

include(../common/common.pri)
#include(../features/openssl/openssl-lin.pri)
include(../features/dirs/wp8/wp8-dirs.pri)
include(../features/cookie-cutter/osdependent/cc-osdependent.pri)
include(../features/osver/wp8/osv-wp8.pri)
#include(../features/mqlib/mqlib.pri)
include(../features/qml/mainwindow/qml-mainwindow.pri)
include(../features/qml/qml5/qmlviewer.pri)

FONTS=

# Default rules for deployment.
include(deployment.pri)
