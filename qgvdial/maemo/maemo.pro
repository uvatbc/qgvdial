TARGET = qgvdial

# Add more folders to ship with the application, here
folder_01.source = qml/maemo
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

# Additional import path used to resolve QML modules in Creators code model
QML_IMPORT_PATH =

QT *= maemo5
CONFIG += mobility
MOBILITY +=

SOURCES  += MainWindow.cpp \
            OsDependant.cpp
HEADERS  += MainWindow.h \
            OsDependant.h

include(../common/common.pri)
include(../../api/api.pri)
include(../features/openssl/openssl.pri)
include(../features/linux-dirs/linux-dirs.pri)

RESOURCES += maemo.qrc

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    qtc_packaging/debian_fremantle/rules \
    qtc_packaging/debian_fremantle/README \
    qtc_packaging/debian_fremantle/copyright \
    qtc_packaging/debian_fremantle/control \
    qtc_packaging/debian_fremantle/compat \
    qtc_packaging/debian_fremantle/changelog
