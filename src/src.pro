QT      *= core gui webkit sql xml xmlpatterns script declarative phonon
TARGET   = qgvdial
TEMPLATE = app

CONFIG  *= precompile_header

include(qtsingleapplication/qtsingleapplication.pri)

CONFIG(debug, debug|release) {
    message(Debug)
    DEFINES += QGV_DEBUG
}

win32 {
CONFIG *= embed_manifest_exe
}

maemo5 {
    message(Maemo5 your world)
    QT *= maemo5
    OTHER_FILES += ../build-files/qgvdial.desktop
}

# In Linux and maemo, add telepathy  and openssl
unix:!symbian: {
    QT *= dbus
    INCLUDEPATH += $$QMAKESPEC/usr/include/telepathy-1.0/
    LIBS += -ltelepathy-qt4 -lssl
}

# In Windows, add openssl
win32 {
    LIBS *= -llibeay32
}

PRECOMPILED_HEADER = ../src/global.h

SOURCES  += ../src/main.cpp                 \
            ../src/MainApp.cpp              \
            ../src/MainWindow.cpp           \
            ../src/Singletons.cpp           \
            ../src/CacheDatabase.cpp        \
            ../src/OsDependent.cpp          \
            ../src/SkypeClientFactory.cpp   \
            ../src/SkypeClient.cpp          \
            ../src/ObserverFactory.cpp      \
            ../src/GVWebPage.cpp            \
            ../src/GVDataAccess.cpp         \
            ../src/GVAccess.cpp             \
            ../src/CalloutInitiator.cpp     \
            ../src/CallInitiatorFactory.cpp \
            ../src/MobileWebPage.cpp        \
            ../src/InboxModel.cpp           \
            ../src/GvXMLParser.cpp          \
            ../src/PhoneNumberValidator.cpp \
            ../src/GVContactsTable.cpp      \
            ../src/CaptchaWidget.cpp        \
            ../src/ContactsXmlHandler.cpp   \
            ../src/SMSEntryDeleteButton.cpp \
            ../src/ChildWindowBase.cpp      \
            ../src/GVInbox.cpp              \
            ../src/WebWidget.cpp            \
            ../src/RegNumberModel.cpp       \
            ../src/ContactsModel.cpp        \
            ../src/ContactDetailsModel.cpp  \
            ../src/DialContext.cpp          \
            ../src/ContactsParserObject.cpp \
            ../src/CookieJar.cpp

HEADERS  += ../src/global.h                 \
            ../src/IObserver.h              \
            ../src/MainApp.h                \
            ../src/MainWindow.h             \
            ../src/Singletons.h             \
            ../src/CacheDatabase.h          \
            ../src/CacheDatabase_int.h      \
            ../src/OsDependent.h            \
            ../src/SkypeClientFactory.h     \
            ../src/SkypeClient.h            \
            ../src/ObserverFactory.h        \
            ../src/GVWebPage.h              \
            ../src/GVDataAccess.h           \
            ../src/GVAccess.h               \
            ../src/CalloutInitiator.h       \
            ../src/CallInitiatorFactory.h   \
            ../src/MobileWebPage.h          \
            ../src/InboxModel.h             \
            ../src/GvXMLParser.h            \
            ../src/PhoneNumberValidator.h   \
            ../src/GVContactsTable.h        \
            ../src/CaptchaWidget.h          \
            ../src/ContactsXmlHandler.h     \
            ../src/SMSEntryDeleteButton.h   \
            ../src/ChildWindowBase.h        \
            ../src/GVInbox.h                \
            ../src/WebWidget.h              \
            ../src/RegNumberModel.h         \
            ../src/ContactsModel.h          \
            ../src/ContactDetailsModel.h    \
            ../src/DialContext.h            \
            ../src/ContactsParserObject.h   \
            ../src/CookieJar.h

RESOURCES += ../src/qgvdial.qrc

# This is so that QtCreator can show these files in the files list.
OTHER_FILES  += ../src/winrsrc.rc           \
                ../qml/About.qml            \
                ../qml/ActionButtons.qml    \
                ../qml/ComboBoxPhones.qml   \
                ../qml/ContactDetails.qml   \
                ../qml/ContactsList.qml     \
                ../qml/DbgWebWidget.qml     \
                ../qml/DialDisp.qml         \
                ../qml/DigitButton.qml      \
                ../qml/ExpandView.qml       \
                ../qml/helper.js            \
                ../qml/HMain.qml            \
                ../qml/InboxList.qml        \
                ../qml/Keypad.qml           \
                ../qml/LoginDetails.qml     \
                ../qml/LogView.qml          \
                ../qml/Main.qml             \
                ../qml/MainView.qml         \
                ../qml/Mosquitto.qml        \
                ../qml/MsgBox.qml           \
                ../qml/MyButton.qml         \
                ../qml/MyTextEdit.qml       \
                ../qml/PinSetting.qml       \
                ../qml/Proxy.qml            \
                ../qml/RadioButton.qml      \
                ../qml/RefreshButtons.qml   \
                ../qml/SaveCancel.qml       \
                ../qml/Scrollbar.qml        \
                ../qml/Settings.qml         \
                ../qml/SmsView.qml          \
                ../qml/Tab.qml              \
                ../qml/TabbedUI.qml         \
                readme.txt

# In Linux and maemo, add the telepathy related sources and headers.
unix:!symbian {
    HEADERS  += ../src/TpObserver.h            \
                ../src/TpCalloutInitiator.h    \
                ../src/QGVDbusServer.h
    SOURCES  += ../src/TpObserver.cpp          \
                ../src/TpCalloutInitiator.cpp  \
                ../src/QGVDbusServer.cpp
}

# In desktop Linux, add the Skype client
unix:!symbian:!maemo5 {
    HEADERS  += ../src/SkypeLinuxClient.h              \
                ../src/SkypeObserver.h                 \
                ../src/DesktopSkypeCallInitiator.h
    SOURCES  += ../src/SkypeLinuxClient.cpp            \
                ../src/SkypeObserver.cpp               \
                ../src/DesktopSkypeCallInitiator.cpp
}

win32 {
# Resource file is for windows only - for the icon
    RC_FILE = ../src/winrsrc.rc

# In desktop Windows, add the Skype client.
    HEADERS += ../src/SkypeWinClient.h             \
               ../src/SkypeObserver.h              \
               ../src/DesktopSkypeCallInitiator.h
    SOURCES += ../src/SkypeWinClient.cpp           \
               ../src/SkypeObserver.cpp            \
               ../src/DesktopSkypeCallInitiator.cpp
}

############################## Mosquitto ##############################
# Add mosquitto support sources to EVERYONE.
    HEADERS  += ../src/MqClientThread.h
    SOURCES  += ../src/MqClientThread.cpp

exists(../src/mqlib-build) {
    message(Forcible inclusion of mqlib!)
    include(../src/mqlib/mqlib.pri)
} else {
    message(Forcible exclusion of mqlib!)
    LIBS += -lmosquitto
}
#######################################################################

symbian {
    HEADERS  += ../src/SymbianCallInitiator.h          \
                ../src/SymbianCallInitiatorPrivate.h   \
                ../src/SymbianCallObserverPrivate.h    \
                ../src/SymbianDTMFPrivate.h
    SOURCES  += ../src/SymbianCallInitiator.cpp        \
                ../src/SymbianCallInitiatorPrivate.cpp \
                ../src/SymbianCallObserverPrivate.cpp  \
                ../src/SymbianDTMFPrivate.cpp

# The Symbian telephony stack library and the equivalent of openssl
    LIBS += -letel3rdparty -llibcrypto

    TARGET.UID3 = 0x2003B499
    TARGET.CAPABILITY += NetworkServices ReadUserData ReadDeviceData SwEvent
    TARGET.EPOCSTACKSIZE = 0x14000          # 80 KB stack size
    TARGET.EPOCHEAPSIZE = 0x020000 0x2000000 # 128 KB - 20 MB

# the icon for our sis file
    ICON=../icons/qgv.svg
# This hack is required until the next version of QT SDK
    QT_CONFIG -= opengl

    vendorinfo = "%{\"Yuvraaj Kelkar\"}" \
                 ":\"Yuvraaj Kelkar\""

    my_deployment.pkg_prerules = vendorinfo
    DEPLOYMENT += my_deployment
}

# For the beta version of the SDK, this part will identify Meego/Harmattan
exists($$QMAKE_INCDIR_QT"/../qmsystem2/qmkeys.h"):!contains(MEEGO_EDITION,harmattan): {
  MEEGO_VERSION_MAJOR     = 1
  MEEGO_VERSION_MINOR     = 2
  MEEGO_VERSION_PATCH     = 0
  MEEGO_EDITION           = harmattan
  DEFINES += MEEGO_HARMATTAN
}

contains(MEEGO_EDITION,harmattan) {
 message(Meego!)
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

    INSTALLS += target desktop icon icon48 icon64 icon_scalable dbusservice

    target.path =$$BINDIR

    desktop.path = $$DATADIR/applications/hildon
    desktop.files += ../build-files/qgvdial.desktop

    icon.path = $$OPTDATADIR
    icon.files += qgvdial.png

    icon48.path = $$DATADIR/icons/hicolor/48x48/hildon
    icon48.files = ../icons/48/qgvdial.png

    icon64.path = $$DATADIR/icons/hicolor/64x64/hildon
    icon64.files = ../icons/64/qgvdial.png

    icon_scalable.path = $$DATADIR/icons/hicolor/scalable/hildon
    icon_scalable.files = qgvdial.png

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
