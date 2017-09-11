QT      *= core network sql xml xmlpatterns
TARGET   = qgvnotify
TEMPLATE = app
INCLUDEPATH += $$PWD/../qgvdial/common/
LIBS    *= -lssl -lcrypto

MOC_DIR = moc
OBJECTS_DIR = obj

CONFIG  *= precompile_header mobility console

PRECOMPILED_HEADER = $$PWD/../qgvdial/common/global.h

SOURCES  += main.cpp                        \
            MainWindow.cpp                  \
            NotifyGVContactsTable.cpp       \
            NotifyGVInbox.cpp               \
            $$PWD/../qgvdial/common/MqClient.cpp

HEADERS  += platform_specific.h             \
            MainWindow.h                    \
            NotifyGVContactsTable.h         \
            NotifyGVInbox.h                 \
            $$PWD/../qgvdial/common/global.h    \
            $$PWD/../qgvdial/common/MqClient.h
            
include($$PWD/../api/api.pri)
include($$PWD/../qgvdial/features/mqlib/mqlib.pri)
