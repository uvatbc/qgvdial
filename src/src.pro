QT      *= core gui webkit sql xml script
TARGET   = qgvdial
TEMPLATE = app

CONFIG  += mobility
MOBILITY = multimedia bearer

include(qtsingleapplication/qtsingleapplication.pri)

win32 {
CONFIG *= embed_manifest_exe
}

maemo5 {
    message("Maemo5 your world")
    QT *= maemo5
    OTHER_FILES += qgvdial.desktop
}

# In Linux and maemo, add the telepathy libraries, sources and headers
unix:!symbian {
    QT *= dbus
    INCLUDEPATH += $$QMAKESPEC/usr/include/telepathy-1.0/
    LIBS += -ltelepathy-qt4
}

SOURCES  += main.cpp                    \
            global.cpp                  \
            MainApp.cpp                 \
            MainWindow.cpp              \
            LoginDialog.cpp             \
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
            GVH_XMLJsonHandler.cpp      \
            DialDigitButton.cpp         \
            PhoneNumberValidator.cpp    \
            GVContactsTable.cpp         \
            CaptchaWidget.cpp           \
            ContactsXmlHandler.cpp      \
            DialCancelDlg.cpp           \
            DlgSelectContactNumber.cpp  \
            SMSEntryDeleteButton.cpp    \
            SMSDlg.cpp                  \
            ChildWindowBase.cpp         \
            GVHistory.cpp \
    ContactsTreeView.cpp \
    InboxTreeView.cpp \
    VMailDialog.cpp

HEADERS  += global.h                    \
            IObserver.h                 \
            MainApp.h                   \
            MainWindow.h                \
            LoginDialog.h               \
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
            GVH_XMLJsonHandler.h        \
            DialDigitButton.h           \
            PhoneNumberValidator.h      \
            GVContactsTable.h           \
            CaptchaWidget.h             \
            ContactsXmlHandler.h        \
            DialCancelDlg.h             \
            DlgSelectContactNumber.h    \
            SMSEntryDeleteButton.h      \
            SMSDlg.h                    \
            ChildWindowBase.h           \
            GVHistory.h \
    ContactsTreeView.h \
    InboxTreeView.h \
    VMailDialog.h

FORMS    += MainWindow.ui               \
            LoginDialog.ui              \
            ContactsWidget.ui           \
            InboxWidget.ui \
    VMailDialog.ui

RESOURCES += qgvdial.qrc

# This is so that QtCreator can show these files in the files list.
OTHER_FILES  += winrsrc.rc

# In Linux and maemo, add the telepathy related sources and headers
unix:!symbian {
    HEADERS  += TpObserver.h            \
                TpCalloutInitiator.h    \
                QGVDbusServer.h
    SOURCES  += TpObserver.cpp          \
                TpCalloutInitiator.cpp  \
                QGVDbusServer.cpp
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

# In desktop Windows, add the Skype client
    HEADERS += SkypeWinClient.h             \
               SkypeObserver.h              \
               DesktopSkypeCallInitiator.h
    SOURCES += SkypeWinClient.cpp           \
               SkypeObserver.cpp            \
               DesktopSkypeCallInitiator.cpp
}

symbian {
    TARGET.UID3 = 0xe2d130b9
    # TARGET.CAPABILITY +=
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}
