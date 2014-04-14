TARGET = qgvdial

# Add more folders to ship with the application, here
folder_01.source = qml/symbian
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

MOC_DIR = moc
OBJECTS_DIR = obj

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

#CONFIG *= mobility
#MOBILITY *= feedback

QT *= phonon

# Add dependency to Symbian components
CONFIG += qt-components

DEFINES += ENABLE_FUZZY_TIMER

INCLUDEPATH += $$PWD
SOURCES  += SymbianPhoneFactory.cpp \
            MainWindow.cpp
HEADERS  += platform_specific.h \
            SymbianPhoneFactory.h \
            MainWindow.h

!simulator {
SOURCES  += SymbianPhoneAccount.cpp \
            SymbianCallInitiatorPrivate.cpp
HEADERS  += SymbianPhoneAccount.h \
            SymbianCallInitiatorPrivate.h

# The Symbian telephony stack library
LIBS += -letel3rdparty
}

include(../common/common.pri)
include(../features/openssl/openssl-symbian.pri)
include(../features/dirs/linux/linux-dirs.pri)
include(../features/qml/qml4/qmlviewer.pri)
include(../features/qml/mainwindow/qml-mainwindow.pri)
include(../features/cookie-cutter/osdependent/cc-osdependent.pri)
include(../features/osver/nokia/symbian/osv-symbian.pri)
include(../features/mqlib/mqlib.pri)

RESOURCES += symbian.qrc

exists(isS1) {
# This hack is required for S^1
QT_CONFIG -= opengl
LIBS -= -lGLESv2
LIBS -= -llibGLESv2
}

#####################################################
# Deployment information
#####################################################
symbian {
TARGET.UID3 = 0x2003B499
TARGET.CAPABILITY += NetworkServices ReadUserData ReadDeviceData WriteDeviceData SwEvent
TARGET.EPOCSTACKSIZE = 0x14000          # 80 KB stack size
TARGET.EPOCHEAPSIZE = 0x020000 0x2000000 # 128 KB - 20 MB
}

# Smart Installer package's UID
# This UID is from the protected range and therefore the package will
# fail to install if self-signed. By default qmake uses the unprotected
# range value if unprotected UID is defined for the application and
# 0x2002CCCF value if protected UID is given to the application
symbian:DEPLOYMENT.installer_header = 0x2002CCCF

vendorinfo = "%{\"Yuvraaj Kelkar\"}" \
             ":\"Yuvraaj Kelkar\""
my_deployment.pkg_prerules = vendorinfo
DEPLOYMENT += my_deployment
#####################################################

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()
