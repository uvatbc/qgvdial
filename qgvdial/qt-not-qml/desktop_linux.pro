include(./common-code.pri)
include(../features/dirs/linux/linux-dirs.pri)
include(../features/openssl/openssl-lin.pri)
#include(../features/skype-desktop/skype.pri)
include(../features/tp/linux/tp.pri)
include(../features/osver/desktop/linux/osv-linux.pri)
include(../features/dbus_api/dbus_api.pri)

# Desktop linux still uses webkit and not webengine even with qt5!!
include($$PWD/../features/webview/webkitview.pri)
#include($$PWD/../features/webview/webengineview.pri)

INCLUDEPATH += desktop_linux
SOURCES  += desktop_linux/PhoneFactory.cpp
HEADERS  += desktop_linux/platform_specific.h \
            desktop_linux/PhoneFactory.h

greaterThan(QT_MAJOR_VERSION, 4) {
QT *= multimedia widgets webkitwidgets
} else {
QT *= phonon
}

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()
