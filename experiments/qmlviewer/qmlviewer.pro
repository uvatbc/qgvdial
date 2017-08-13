TARGET=qmlviewer
TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .

QT += declarative

MOC_DIR = moc
OBJECTS_DIR = obj

HEADERS  += global.h        \
            MainWindow.h    \


SOURCES  += main.cpp        \
            MainWindow.cpp  \


include($$PWD/../../qgvdial/features/qml/qml4/qmlviewer.pri)
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()
