include(./common-code.pri)
include(../features/skype-desktop/skype.pri)

# Surprisingly enough, this works just perfectly in Windows
include(../features/dirs/linux/linux-dirs.pri)

INCLUDEPATH += desktop_windows
SOURCES  += desktop_windows/MainApp.cpp \
            desktop_windows/PhoneFactory.cpp \
            desktop_windows/ObserverFactory.cpp
HEADERS  += desktop_windows/MainApp.h \
            desktop_windows/PhoneFactory.h \
            desktop_windows/ObserverFactory.h

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()
