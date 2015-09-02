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
            src/MainWindow.h \
            src/SailfishPhoneFactory.h

SOURCES  += src/MainWindow.cpp \
            src/SailfishPhoneFactory.cpp

RESOURCES += sailfish.qrc

OTHER_FILES  += rpm/qgvdial.spec \
                rpm/sailfish.yaml \
                qgvdial.desktop

OTHER_FILES  += qml/cover/CoverPage.qml \
                qml/pages/FirstPage.qml \
                qml/pages/SecondPage.qml \
                qml/qgvdial.qml \
                qml/pages/DialPage.qml \
                qml/pages/ContactsPage.qml \
                qml/pages/RefreshButton.qml \
                qml/pages/InboxPage.qml \
                qml/pages/InboxDetailsPage.qml \
                qml/pages/SettingsPage.qml \
                qml/pages/ExpandView.qml \
                qml/pages/LoginDetails.qml \
                qml/pages/Proxy.qml \
                qml/pages/Updates.qml \
                qml/pages/EtCetera.qml \
                qml/pages/TfaPinPage.qml \
                qml/pages/RegNumberSelector.qml \
                qml/pages/ContactDetailsPage.qml \
                qml/pages/MessageBox.qml \
                qml/pages/CiPhoneSelectionPage.qml \
                qml/pages/SmsPage.qml \
                qml/pages/WebPage.qml \
                qml/pages/StatusBanner.qml

QT *= multimedia

include(../common/common.pri)
include(../features/openssl/openssl-lin.pri)
include(../features/dirs/linux/linux-dirs.pri)
include(../features/dbus_api/dbus_api.pri)
include(../features/qml/qml5-sailfish/qmlviewer.pri)
include(../features/qml/mainwindow/qml-mainwindow.pri)
include(../features/cookie-cutter/osdependent/cc-osdependent.pri)
include(../features/osver/sailfish/osv-sailfish.pri)

include(../features/tp/linux/tp.pri)
include(../features/mqlib/mqlib.pri)
