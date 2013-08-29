include(./common-code.pri)

#TODO: Change this!
include(../features/linux-dirs/linux-dirs.pri)

SOURCES += WinMainApp.cpp
HEADERS += WinMainApp.h

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()
