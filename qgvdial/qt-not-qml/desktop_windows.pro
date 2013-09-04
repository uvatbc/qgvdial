include(./common-code.pri)

#TODO: Change this!
include(../features/linux-dirs/linux-dirs.pri)

INCLUDEPATH += desktop_windows
SOURCES  += WinMainApp.cpp \
            desktop_windows/PhoneFactory.cpp
HEADERS  += WinMainApp.h \
            desktop_windows/PhoneFactory.h

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()
