QT      *= core gui webkit sql xml xmlpatterns script declarative
TARGET   = qgvdial
TEMPLATE = app

CONFIG  *= precompile_header mobility
MOBILITY *= multimedia

include(qtsingleapplication/qtsingleapplication.pri)

win32 {
CONFIG *= embed_manifest_exe
}

maemo5 {
    message(Maemo5 your world)
    QT *= maemo5
    OTHER_FILES += ../build-files/qgvdial.desktop
}

# In Linux and maemo, add the telepathy libraries, headers and libmosquitto
unix:!symbian {
    QT *= dbus
    INCLUDEPATH += $$QMAKESPEC/usr/include/telepathy-1.0/
    LIBS += -ltelepathy-qt4 -lmosquitto -leay32
}
# In Windows, add the mosquitto dll
win32 {
    LIBS *= -lmosquitto -llibeay32
}

PRECOMPILED_HEADER = global.h

SOURCES  += main.cpp                    \
            MainApp.cpp                 \
            MainWindow.cpp              \
            Singletons.cpp              \
            CacheDatabase.cpp           \
            OsDependent.cpp             \
            SkypeClientFactory.cpp      \
            SkypeClient.cpp             \
            ObserverFactory.cpp         \
            GVWebPage.cpp               \
            GVDataAccess.cpp            \
            GVAccess.cpp                \
            CalloutInitiator.cpp        \
            CallInitiatorFactory.cpp    \
            MobileWebPage.cpp           \
            InboxModel.cpp              \
            GVI_XMLJsonHandler.cpp      \
            PhoneNumberValidator.cpp    \
            GVContactsTable.cpp         \
            CaptchaWidget.cpp           \
            ContactsXmlHandler.cpp      \
            SMSEntryDeleteButton.cpp    \
            SMSDlg.cpp                  \
            ChildWindowBase.cpp         \
            GVInbox.cpp                 \
            WebWidget.cpp               \
            RegNumberModel.cpp          \
            ContactsModel.cpp           \
            ContactDetailsModel.cpp     \
            GVI_SMS_Handler.cpp         \
            DialContext.cpp             \
            ContactsParserObject.cpp

HEADERS  += global.h                    \
            IObserver.h                 \
            MainApp.h                   \
            MainWindow.h                \
            Singletons.h                \
            CacheDatabase.h             \
            OsDependent.h               \
            SkypeClientFactory.h        \
            SkypeClient.h               \
            ObserverFactory.h           \
            GVWebPage.h                 \
            GVDataAccess.h              \
            GVAccess.h                  \
            CalloutInitiator.h          \
            CallInitiatorFactory.h      \
            MobileWebPage.h             \
            InboxModel.h                \
            GVI_XMLJsonHandler.h        \
            PhoneNumberValidator.h      \
            GVContactsTable.h           \
            CaptchaWidget.h             \
            ContactsXmlHandler.h        \
            SMSEntryDeleteButton.h      \
            SMSDlg.h                    \
            ChildWindowBase.h           \
            GVInbox.h                   \
            WebWidget.h                 \
            RegNumberModel.h            \
            ContactsModel.h             \
            ContactDetailsModel.h       \
            GVI_SMS_Handler.h           \
            DialContext.h               \
            ContactsParserObject.h

RESOURCES += qgvdial.qrc

# This is so that QtCreator can show these files in the files list.
OTHER_FILES  += winrsrc.rc                  \
                ../qml/About.qml            \
                ../qml/ActionButtons.qml    \
                ../qml/ComboBoxPhones.qml   \
                ../qml/ContactDetails.qml   \
                ../qml/ContactsList.qml     \
                ../qml/DbgWebWidget.qml     \
                ../qml/DialDisp.qml         \
                ../qml/DigitButton.qml      \
                ../qml/helper.js            \
                ../qml/InboxList.qml        \
                ../qml/Keypad.qml           \
                ../qml/LogView.qml          \
                ../qml/MainButtons.qml      \
                ../qml/Main.qml             \
                ../qml/MainView.qml         \
                ../qml/Mosquitto.qml        \
                ../qml/MsgBox.qml           \
                ../qml/MyButton.qml         \
                ../qml/MyTextEdit.qml       \
                ../qml/Proxy.qml            \
                ../qml/RadioButton.qml      \
                ../qml/Settings.qml         \
                readme.txt

# In Linux and maemo, add the telepathy related sources and headers. Also add Mosquitto based sources and headers.
unix:!symbian {
    HEADERS  += TpObserver.h            \
                TpCalloutInitiator.h    \
                QGVDbusServer.h         \
                MqClientThread.h
    SOURCES  += TpObserver.cpp          \
                TpCalloutInitiator.cpp  \
                QGVDbusServer.cpp       \
                MqClientThread.cpp
}

# In desktop Linux, add the Skype client
unix:!symbian:!maemo5 {
    HEADERS  += SkypeLinuxClient.h              \
                SkypeObserver.h                 \
                DesktopSkypeCallInitiator.h
    SOURCES  += SkypeLinuxClient.cpp            \
                SkypeObserver.cpp               \
                DesktopSkypeCallInitiator.cpp
}

win32 {
# Resource file is for windows only - for the icon
    RC_FILE = winrsrc.rc

# In desktop Windows, add the Skype client. Also add Mosquitto based sources and headers.
    HEADERS += SkypeWinClient.h             \
               SkypeObserver.h              \
               DesktopSkypeCallInitiator.h  \
               MqClientThread.h
    SOURCES += SkypeWinClient.cpp           \
               SkypeObserver.cpp            \
               DesktopSkypeCallInitiator.cpp \
               MqClientThread.cpp
}

symbian {
    HEADERS += SymbianCallInitiator.h           \
               SymbianCallInitiatorPrivate.h    \
               SymbianCallObserverPrivate.h
    SOURCES += SymbianCallInitiator.cpp         \
               SymbianCallInitiatorPrivate.cpp  \
               SymbianCallObserverPrivate.cpp

# Add the mosquitto lib to symbian
    include(mqlib/mqlib.pri)
    HEADERS += MqClientThread.h
    SOURCES += MqClientThread.cpp

# The Symbian telephony stack library
    LIBS += -letel3rdparty -llibcrypto

    TARGET.UID3 = 0x2003B499
    TARGET.CAPABILITY += NetworkServices ReadUserData ReadDeviceData SwEvent
    TARGET.EPOCSTACKSIZE = 0x14000          # 80 KB stack size
    TARGET.EPOCHEAPSIZE = 0x020000 0x2000000 # 128 KB - 20 MB

# the icon for our sis file
    ICON=../icons/Google.svg
# This hack is required until the next version of QT SDK
    QT_CONFIG -= opengl
}

###############################################################
# Installation related line go here
###############################################################

# Installation for maemo
maemo5 {
    exists(../../buildit.sh) {
        PREFIX = ../debian/qgvdial/usr
        message(Built using my scripts... probably inside scratchbox)
    }
    exists(../../buildit.pl) {
        PREFIX = ../debian/qgvdial/usr
        message(Built using my scripts)
    }
    !exists(../../buildit.pl):!exists(../../buildit.sh) {
        PREFIX = ../maemo/debian/qgvdial/usr
        message(Build using qtcreator)
    }

    message(maemo5 install)
    OPTPREFIX  = $$PREFIX/../opt/qgvdial
    BINDIR     = $$OPTPREFIX/bin
    DATADIR    = $$PREFIX/share
    OPTDATADIR = $$OPTPREFIX/share

    DEFINES += DATADIR=\"$$DATADIR\" PKGDATADIR=\"$$PKGDATADIR\"

    INSTALLS += target desktop icon dbusservice

    target.path =$$BINDIR

    desktop.path = $$DATADIR/applications/hildon
    desktop.files += ../build-files/qgvdial.desktop

    icon.path = $$OPTDATADIR
    icon.files += qgvdial.png

    dbusservice.path = $$DATADIR/dbus-1/services
    dbusservice.files += ../build-files/qgvdial.Call.service
    dbusservice.files += ../build-files/qgvdial.Text.service
}

# Installation for Linux
unix:!symbian:!maemo5 {
    BINDIR  = $$PREFIX/bin/qgvdial
    DATADIR = $$PREFIX/share
    message($$BINDIR)

    DEFINES += DATADIR=\"$$DATADIR\" PKGDATADIR=\"$$PKGDATADIR\"

    INSTALLS += target icon dbusservice

    target.path =$$BINDIR

    icon.path = $$DATADIR/qgvdial/icons
    icon.files += qgvdial.png

    dbusservice.path = $$DATADIR/dbus-1/services
    dbusservice.files += ../build-files/qgvdial.Call.service
    dbusservice.files += ../build-files/qgvdial.Text.service
}
