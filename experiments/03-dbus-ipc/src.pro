TARGET = 03_dbus
TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .

MOC_DIR = moc
OBJECTS_DIR = obj

QT += dbus

PRECOMPILED_HEADER = global.h

# Input
HEADERS  += global.h        \
            MainWindow.h    \
            qgvn_adapter.h  \
            qgvn_proxy.h    \
            CtrlService.h

SOURCES  += main.cpp        \
            MainWindow.cpp  \
            qgvn_adapter.cpp \
            qgvn_proxy.cpp  \
            CtrlService.cpp

