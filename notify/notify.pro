QT      *= core gui network webkit sql xml xmlpatterns script
TARGET   = qgvnotify
TEMPLATE = app
INCLUDEPATH=../src

CONFIG  *= precompile_header mobility console

# In Windows, add the mosquitto dll
win32 {
    CONFIG *= embed_manifest_exe
    LIBS *= -lmosquitto -llibeay32
}

unix:!symbian {
    QT *= dbus
    LIBS *= -lmosquitto -lssl

    maemo5 {
        target.path = /opt/usr/bin
    } else {
        target.path = /usr/local/bin
    }
    INSTALLS += target
}

PRECOMPILED_HEADER = ../src/global.h

SOURCES  += main.cpp                        \
            MainWindow.cpp                  \
            NotifySingletons.cpp            \
            NotifyGVContactsTable.cpp       \
            NotifyGVInbox.cpp               \
            ../src/GVAccess.cpp             \
            ../src/GVWebPage.cpp            \
            ../src/MobileWebPage.cpp        \
            ../src/GVI_XMLJsonHandler.cpp   \
            ../src/GVI_SMS_Handler.cpp      \
            ../src/ContactsParserObject.cpp \
            ../src/ContactsXmlHandler.cpp


HEADERS  += ../src/global.h                 \
            MainWindow.h                    \
            NotifySingletons.h              \
            NotifyGVContactsTable.h         \
            NotifyGVInbox.h                 \
            ../src/GVAccess.h               \
            ../src/GVWebPage.h              \
            ../src/MobileWebPage.h          \
            ../src/GVI_XMLJsonHandler.h     \
            ../src/GVI_SMS_Handler.h        \
            ../src/ContactsParserObject.h   \
            ../src/ContactsXmlHandler.h
