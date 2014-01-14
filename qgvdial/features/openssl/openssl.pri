INCLUDEPATH += $$PWD

SOURCES  += $$PWD/OSDCipher.cpp
HEADERS  += $$PWD/OSDCipher.h

win32 {
    LIBS *= -llibeay32
} else {
# For blackberry, add the openssl library:
blackberry {
    LIBS += -lssl -lcrypto
} else {
symbian {
LIBS += -llibcrypto
} else {
# For all Linux variants, add openssl
    LIBS += -lssl -lcrypto
} } }
