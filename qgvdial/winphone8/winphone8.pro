TEMPLATE = app
TARGET = qgvdial

QT += qml quick widgets multimedia

SOURCES  += MainWindow.cpp \

HEADERS  += platform_specific.h \
            MainWindow.h \


RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = qml

include(../common/common.pri)
#include(../features/openssl/openssl-lin.pri)
#include(../features/dirs/bb10/bb10-dirs.pri)
include(../features/cookie-cutter/osdependent/cc-osdependent.pri)
#include(../features/osver/bb10/osv-bb10.pri)
include(../features/mqlib/mqlib.pri)
include(../features/qml/mainwindow/qml-mainwindow.pri)
include(../features/qml/qml5/qmlviewer.pri)

FONTS=

# Default rules for deployment.
include(deployment.pri)
