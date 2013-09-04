TARGET=qgvdial

# Add files and directories to ship with the application
# by adapting the examples below.
# file1.source = myfile
# dir1.source = mydir
DEPLOYMENTFOLDERS = # file1 dir1

# If your application uses the Qt Mobility libraries, uncomment
# the following lines and add the respective components to the
# MOBILITY variable.
# CONFIG += mobility
# MOBILITY +=

SOURCES  += MainWindow.cpp \
            MainWindow_p.cpp \
            OsDependant.cpp
HEADERS  += MainWindow.h \
            MainWindow_p.h \
            OsDependant.h

FORMS   += mainwindow.ui
RESOURCES += qtnotqml.qrc

include(../common/common.pri)
include(../../api/api.pri)
include(../features/openssl/openssl.pri)
include(../features/qtsingleapplication/qtsingleapplication.pri)
