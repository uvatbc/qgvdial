TARGET = qgvdial

# Add more folders to ship with the application, here
folder_01.source = qml/bb10
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

MOC_DIR = moc
OBJECTS_DIR = obj

# Additional import path used to resolve QML modules in Creators code model
QML_IMPORT_PATH = ./imports

# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
#CONFIG += mobility
#MOBILITY +=

# Speed up launching on MeeGo/Harmattan when using applauncherd daemon
#CONFIG += qdeclarative-boostable

# Add dependency to Symbian components
# CONFIG += qt-components

LIBS += -lbbsystem

SOURCES  += BB10PhoneFactory.cpp \
            BBPhoneAccount.cpp
HEADERS  += platform_specific.h \
            BB10PhoneFactory.h \
            BBPhoneAccount.h

include(../common/common.pri)
include(../../api/api.pri)
include(../features/openssl/openssl.pri)
include(../features/dirs/bb10/bb10-dirs.pri)
include(../features/qml/mainwindow/qml-mainwindow.pri)
include(../features/cookie-cutter/osdependent/cc-osdependent.pri)
include(../features/osver/bb10/osv-bb10.pri)

RESOURCES += bb10.qrc

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    bar-descriptor.xml
