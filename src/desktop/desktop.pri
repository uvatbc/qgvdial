INCLUDEPATH += $$PWD

# In desktop Linux, add the Skype client
unix:!symbian:!maemo5 {
    HEADERS  += $$PWD/SkypeLinuxClient.h           \
                $$PWD/SkypeObserver.h              \
                $$PWD/DesktopSkypeCallInitiator.h

    SOURCES  += $$PWD/SkypeLinuxClient.cpp         \
                $$PWD/SkypeObserver.cpp            \
                $$PWD/DesktopSkypeCallInitiator.cpp

}

win32 {
    CONFIG *= embed_manifest_exe

# Resource file is for windows only - for the icon
    RC_FILE = $$PWD/winrsrc.rc

# In desktop Windows, add the Skype client.
    HEADERS += $$PWD/SkypeWinClient.h             \
               $$PWD/SkypeObserver.h              \
               $$PWD/DesktopSkypeCallInitiator.h

    SOURCES += $$PWD/SkypeWinClient.cpp           \
               $$PWD/SkypeObserver.cpp            \
               $$PWD/DesktopSkypeCallInitiator.cpp

# In Windows, add openssl
    LIBS *= -llibeay32
}

# Installation for Linux
unix:!symbian:!maemo5:!contains(DEFINES,MEEGO_HARMATTAN) {
    DATADIR = /usr/share
    message(Regular Linux install)

    INSTALLS += target icon dbusservice

    target.path =/usr/bin/qgvdial

    icon.path = $$DATADIR/qgvdial/icons
    icon.files += qgvdial.png

    dbusservice.path = $$DATADIR/dbus-1/services
    dbusservice.files = ../build-files/qgvdial/ubuntu/qgvdial.Call.service \
                        ../build-files/qgvdial/ubuntu/qgvdial.Text.service
}

