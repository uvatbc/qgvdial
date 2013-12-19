TARGET = qgvdial

# Add more folders to ship with the application, here
folder_01.source = qml/bb10
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

MOC_DIR = moc
OBJECTS_DIR = obj

# Additional import path used to resolve QML modules in Creators code model
QML_IMPORT_PATH = ./imports

CONFIG *= mobility
MOBILITY *= multimedia

# Speed up launching on MeeGo/Harmattan when using applauncherd daemon
#CONFIG += qdeclarative-boostable

# Add dependency to Symbian components
# CONFIG += qt-components

SOURCES  += BB10PhoneFactory.cpp
HEADERS  += platform_specific.h \
            BB10PhoneFactory.h
RESOURCES += bb10.qrc

# So that I can open this project in the Qt SDK and have it compile correctly:
!simulator {
SOURCES  += BBPhoneAccount.cpp
HEADERS  += BBPhoneAccount.h
LIBS += -lbbsystem
}

include(../common/common.pri)
include(../../api/api.pri)
include(../features/openssl/openssl.pri)
include(../features/dirs/bb10/bb10-dirs.pri)
include(../features/qml/mainwindow/qml-mainwindow.pri)
include(../features/cookie-cutter/osdependent/cc-osdependent.pri)
include(../features/osver/bb10/osv-bb10.pri)

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    bar-descriptor.xml
