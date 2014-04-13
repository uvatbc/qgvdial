include(openssl.pri)

symbian {
    LIBS += -llibssl -llibcrypto
} else {
# For the simulator:
    LIBS += -lssl -lcrypto
}
