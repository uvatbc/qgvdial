INCLUDEPATH += $$PWD

SOURCES  += $$PWD/OSDCipher.cpp
HEADERS  += $$PWD/OSDCipher.h

win32 {
    LIBS *= -llibeay32
} else {
symbian {
    LIBS += -llibssl -llibcrypto
} else {
# For all Linux variants, and blackberry: add openssl
    LIBS += -lssl -lcrypto
} }

contains(DEFINES,OPENSSL_STATIC) {
INCLUDEPATH += $$PWD/include
}
