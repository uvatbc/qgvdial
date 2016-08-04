include(./common-code.pri)
include(../features/dirs/linux/linux-dirs.pri)
include(../features/openssl/openssl-lin.pri)
#include(../features/skype-desktop/skype.pri)
include(../features/osver/desktop/mac/osv-mac.pri)

# Mac uses qt5 and webengine
include($$PWD/../features/webview/webengineview.pri)

INCLUDEPATH += desktop_mac
SOURCES  += desktop_mac/PhoneFactory.cpp
HEADERS  += desktop_mac/platform_specific.h \
            desktop_mac/PhoneFactory.h

QT *= multimedia

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()
