# The name of your app.
# NOTICE: name defined in TARGET has a corresponding QML filename.
#         If name defined in TARGET is changed, following needs to be
#         done to match new name:
#         - corresponding QML filename must be changed
#         - desktop icon filename must be changed
#         - desktop filename must be changed
#         - icon definition filename in desktop file must be changed
TARGET = qgvdial

CONFIG += sailfishapp

INCLUDEPATH += src
HEADERS  += src/platform_specific.h \
            src/MainWindow.h
SOURCES  += src/MainWindow.cpp

OTHER_FILES  += rpm/sailfish.spec \
                rpm/sailfish.yaml \
                qgvdial.desktop

OTHER_FILES  += qml/cover/CoverPage.qml \
                qml/pages/FirstPage.qml \
                qml/pages/SecondPage.qml \
                qml/qgvdial.qml

DEFINES += OPENSSL_STATIC

include(../common/common.pri)
include(../features/openssl/openssl.pri)
include(../features/dirs/linux/linux-dirs.pri)
include(../features/dbus_api/dbus_api.pri)
include(../features/dbus_api/dbus_api.pri)
include(../features/qml/qml5-sailfish/qmlviewer.pri)
include(../features/qml/mainwindow/qml-mainwindow.pri)
