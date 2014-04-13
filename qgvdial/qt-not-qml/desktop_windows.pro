include(common-code.pri)
include(../features/osver/desktop/windows/osv-windows.pri)
include(../features/openssl/openssl-win.pri)

# Surprisingly enough, this works just perfectly in Windows
include(../features/dirs/linux/linux-dirs.pri)

PRECOMPILED_HEADER *= desktop_windows/platform_specific.h

INCLUDEPATH += desktop_windows
SOURCES  += desktop_windows/PhoneFactory.cpp
HEADERS  += desktop_windows/platform_specific.h \
            desktop_windows/PhoneFactory.h

#SOURCES  += desktop_windows/MainApp.cpp \
#            desktop_windows/ObserverFactory.cpp
#HEADERS  += desktop_windows/MainApp.h \
#            desktop_windows/ObserverFactory.h
#include(../features/skype-desktop/skype.pri)

greaterThan(QT_MAJOR_VERSION, 4) {
QT *= multimedia
} else {
QT *= phonon
}

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()
