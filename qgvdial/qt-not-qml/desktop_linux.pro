include(./common-code.pri)
include(../features/dirs/linux/linux-dirs.pri)
include(../features/skype-desktop/skype.pri)

INCLUDEPATH += desktop_linux
SOURCES  += desktop_linux/PhoneFactory.cpp
HEADERS  += desktop_linux/platform_specific.h \
            desktop_linux/PhoneFactory.h

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()
