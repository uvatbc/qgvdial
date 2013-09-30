include(./common-code.pri)
include(../features/dirs/linux/linux-dirs.pri)

DEFINES += OS_DIABLO

INCLUDEPATH += diablo
SOURCES  += diablo/DiabloPhoneFactory.cpp
HEADERS  += diablo/platform_specific.h \
            diablo/DiabloPhoneFactory.h

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()

OTHER_FILES += \
    qtc_packaging/debian_fremantle/rules \
    qtc_packaging/debian_fremantle/README \
    qtc_packaging/debian_fremantle/copyright \
    qtc_packaging/debian_fremantle/control \
    qtc_packaging/debian_fremantle/compat \
    qtc_packaging/debian_fremantle/changelog
