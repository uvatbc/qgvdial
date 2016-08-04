include(./common-code.pri)
include(../features/dirs/linux/linux-dirs.pri)
include(../features/openssl/openssl-lin.pri)
include(../features/osver/nokia/diablo/osv-diablo.pri)
include(../features/dbus_api/dbus_api.pri)

# Diablo uses qt4 and webkit
include($$PWD/../features/webview/webkitview.pri)

DEFINES += OS_DIABLO
QT *= phonon

simulator {
DEFINES+=QT_WS_SIMULATOR
}

INCLUDEPATH += diablo
SOURCES  += diablo/DiabloPhoneFactory.cpp
HEADERS  += diablo/platform_specific.h \
            diablo/DiabloPhoneFactory.h

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()

OTHER_FILES += \
    qtc_packaging/debian_fremantle/rules \
    qtc_packaging/debian_fremantle/README \
    qtc_packaging/debian_fremantle/copyright \
    qtc_packaging/debian_fremantle/control \
    qtc_packaging/debian_fremantle/compat \
    qtc_packaging/debian_fremantle/changelog
