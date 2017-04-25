QT      *= core network sql xml xmlpatterns dbus
LIBS	*= -lmosquitto -lssl -lcrypto
TARGET   = qgvnotify
TEMPLATE = app
INCLUDEPATH += $$PWD/../qgvdial/common/

MOC_DIR = moc
OBJECTS_DIR = obj

CONFIG  *= precompile_header mobility console

PRECOMPILED_HEADER = $$PWD/../qgvdial/common/global.h

SOURCES  += main.cpp                        \
            MainWindow.cpp                  \
            NotifyGVContactsTable.cpp       \
            NotifyGVInbox.cpp               \
            MqPublisher.cpp                 \
            dbusinterface/qgvn_adapter.cpp  \
            dbusinterface/qgvn_proxy.cpp

HEADERS  += $$PWD/../qgvdial/common/global.h    \
            platform_specific.h                 \
            MainWindow.h                        \
            NotifyGVContactsTable.h             \
            NotifyGVInbox.h                     \
            MqPublisher.h                       \
            dbusinterface/qgvn_adapter.h        \
            dbusinterface/qgvn_proxy.h

include($$PWD/../api/api.pri)
include($$PWD/../qgvdial/features/mqlib/mqlib.pri)
