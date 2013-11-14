TARGET = qgvdial

# Add more folders to ship with the application, here
folder_01.source = qml/maemo
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

MOC_DIR = moc
OBJECTS_DIR = obj

# Additional import path used to resolve QML modules in Creators code model
QML_IMPORT_PATH =

QT *= maemo5
CONFIG += mobility
MOBILITY +=

SOURCES  += MainWindow.cpp \
            MaemoPhoneFactory.cpp
HEADERS  += platform_specific.h \
            MainWindow.h \
            MaemoPhoneFactory.h

include(../common/common.pri)
include(../../api/api.pri)
include(../features/openssl/openssl.pri)
include(../features/dirs/linux/linux-dirs.pri)
include(../features/dbus_api/dbus_api.pri)
include(../features/qtsingleapplication/qtsingleapplication.pri)
include(../features/cookie-cutter/osdependent/cc-osdependent.pri)

simulator {
include(../features/tp/linux/tp.pri)
} else {
include(../features/tp/maemo/tp.pri)
}

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
