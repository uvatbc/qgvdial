TEMPLATE = app
TARGET   = qgvdial
QT      *= core gui webkit sql
INCLUDEPATH += .

CONFIG  += mobility
MOBILITY = multimedia

win32 {
CONFIG *= embed_manifest_exe
}

maemo5 {
    message("Maemo5 your world")
    QT *= maemo5
}

# Input
HEADERS +=  MainWindow.h                \
            MyWebView.h                 \
            DialerWidget.h              \
            DigitButton.h               \
            MobileWebPage.h             \
            global.h                    \
            GVWebPage.h                 \
            DlgSelectContactNumber.h    \
            GVSettings.h                \
            GVContactsTable.h           \
            OsDependent.h               \
            GVHistory.h                 \
            SMSDlg.h                    \
            SMSEntryDeleteButton.h      \
            VoicemailWidget.h           \
            DialerValidator.h           \
            CacheDatabase.h             \
            ChildWindowBase.h           \
            UniqueAppHelper.h           \
            DialCancelDlg.h             \
            ObserverFactory.h           \
            IObserver.h


SOURCES +=  main.cpp                    \
            MainWindow.cpp              \
            MyWebView.cpp               \
            DialerWidget.cpp            \
            DigitButton.cpp             \
            MobileWebPage.cpp           \
            global.cpp                  \
            GVWebPage.cpp               \
            DlgSelectContactNumber.cpp  \
            GVSettings.cpp              \
            GVContactsTable.cpp         \
            OsDependent.cpp             \
            GVHistory.cpp               \
            SMSDlg.cpp                  \
            SMSEntryDeleteButton.cpp    \
            VoicemailWidget.cpp         \
            DialerValidator.cpp         \
            CacheDatabase.cpp           \
            ChildWindowBase.cpp         \
            UniqueAppHelper.cpp         \
            DialCancelDlg.cpp           \
            ObserverFactory.cpp

RESOURCES = qgvdial.qrc

# Resource file is for windows only - for the icon
win32 {
RC_FILE = winrsrc.rc
}

# In Linux and maemo, add the telepathy libraries, sources and headers
unix {
    QT *= dbus
    INCLUDEPATH += /usr/include/telepathy-1.0
    LIBS += -ltelepathy-qt4

    HEADERS += TpObserver.h
    SOURCES += TpObserver.cpp
}

OTHER_FILES += qgvdial.desktop
