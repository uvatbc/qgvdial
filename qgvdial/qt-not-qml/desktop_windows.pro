include($$PWD/common-code.pri)

# Surprisingly enough, this works just perfectly in Windows
include($$PWD/../features/dirs/linux/linux-dirs.pri)

INCLUDEPATH += $$PWD/desktop_windows
SOURCES  += $$PWD/desktop_windows/PhoneFactory.cpp
HEADERS  += $$PWD/desktop_windows/platform_specific.h \
            $$PWD/desktop_windows/PhoneFactory.h

#SOURCES  += $$PWD/desktop_windows/MainApp.cpp \
#            $$PWD/desktop_windows/ObserverFactory.cpp
#HEADERS  += $$PWD/desktop_windows/MainApp.h \
#            $$PWD/desktop_windows/ObserverFactory.h
#include($$PWD/../features/skype-desktop/skype.pri)

# Please do not modify the following two lines. Required for deployment.
include($$PWD/deployment.pri)
qtcAddDeployment()
