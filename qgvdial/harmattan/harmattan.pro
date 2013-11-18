TARGET = qgvdial

# Add more folders to ship with the application, here
folder_01.source = qml/harmattan
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

MOC_DIR = moc
OBJECTS_DIR = obj

# Additional import path used to resolve QML modules in Creators code model
QML_IMPORT_PATH =

# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
CONFIG += mobility
MOBILITY +=

# Speed up launching on MeeGo/Harmattan when using applauncherd daemon
CONFIG += qdeclarative-boostable

# Add dependency to Symbian components
# CONFIG += qt-components

INCLUDEPATH += $$PWD
SOURCES  += HarmattanPhoneFactory.cpp
HEADERS  += platform_specific.h \
            HarmattanPhoneFactory.h

include(../common/common.pri)
include(../../api/api.pri)
include(../features/openssl/openssl.pri)
include(../features/dirs/linux/linux-dirs.pri)
include(../features/qtsingleapplication/qtsingleapplication.pri)
include(../features/dbus_api/dbus_api.pri)
include(../features/qml/mainwindow/qml-mainwindow.pri)
include(../features/cookie-cutter/osdependent/cc-osdependent.pri)
include(../features/osver/nokia/harmattan/osv-harmattan.pri)

simulator {
include(../features/tp/linux/tp.pri)
} else {
include(../features/tp/maemo/tp.pri)
}

RESOURCES += harmattan.qrc

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/manifest.aegis \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog
