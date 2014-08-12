QT *= sql network
QT -= gui

TARGET = qt4srv
TEMPLATE = app

SOURCES  += main.cpp \
            Srv.cpp

HEADERS  += Srv.h

LIBS += -lbbsystem

MOC_DIR = moc
OBJECTS_DIR = obj

unix {
    target.path = /usr/lib
    INSTALLS += target
}

