# Add more folders to ship with the application, here
folder_01.source = qml/symbian
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

symbian:TARGET.UID3 = 0xEB6B03F9

# Smart Installer package's UID
# This UID is from the protected range and therefore the package will
# fail to install if self-signed. By default qmake uses the unprotected
# range value if unprotected UID is defined for the application and
# 0x2002CCCF value if protected UID is given to the application
#symbian:DEPLOYMENT.installer_header = 0x2002CCCF

# Allow network access on Symbian
symbian:TARGET.CAPABILITY += NetworkServices

# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
CONFIG += mobility
MOBILITY +=

QT *= phonon

# Add dependency to Symbian components
CONFIG += qt-components

SOURCES  += MainWindow.cpp \
            SymbianPhoneFactory.cpp
HEADERS  += platform_specific.h \
            MainWindow.h \
            SymbianPhoneFactory.h

include(../common/common.pri)
include(../../api/api.pri)
include(../features/openssl/openssl.pri)
include(../features/dirs/linux/linux-dirs.pri)
include(../features/dummy/dummy.pri)
include(../features/cookie-cutter/osdependent/cc-osdependent.pri)

# Lets pretend that the dummy is a variant of Linux
include(../features/osver/desktop/linux/osv-linux.pri)

RESOURCES += symbian.qrc

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()
