INCLUDEPATH += $$PWD
CONFIG += qt-components
MOBILITY *= feedback

HEADERS  += $$PWD/SymbianCallInitiator.h          \
            $$PWD/SymbianCallInitiatorPrivate.h   \
            $$PWD/SymbianCallObserverPrivate.h    \
            $$PWD/SymbianDTMFPrivate.h
SOURCES  += $$PWD/SymbianCallInitiator.cpp        \
            $$PWD/SymbianCallInitiatorPrivate.cpp \
            $$PWD/SymbianCallObserverPrivate.cpp  \
            $$PWD/SymbianDTMFPrivate.cpp

# The Symbian telephony stack library and the equivalent of openssl
LIBS += -letel3rdparty -llibcrypto

TARGET.UID3 = 0x2003B499
TARGET.CAPABILITY += NetworkServices ReadUserData ReadDeviceData WriteDeviceData SwEvent
TARGET.EPOCSTACKSIZE = 0x14000          # 80 KB stack size
TARGET.EPOCHEAPSIZE = 0x020000 0x2000000 # 128 KB - 20 MB

# the icon for our sis file
ICON=$$PWD/../../icons/qgv.svg
# This hack is required until the next version of QT SDK
QT_CONFIG -= opengl

vendorinfo = "%{\"Yuvraaj Kelkar\"}" \
             ":\"Yuvraaj Kelkar\""

my_deployment.pkg_prerules = vendorinfo

contains(CONFIG,qt-components) {
# This makes the smart installer get qt-components.... I think
DEPLOYMENT.installer_header = 0x2002CCCF
my_deployment.pkg_prerules += \
    "; Dependency to Symbian Qt Quick components" \
    "(0x200346DE), 1, 0, 0, {\"Qt Quick components for Symbian\"}"
}

DEPLOYMENT += my_deployment

