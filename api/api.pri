INCLUDEPATH += $$PWD

SOURCES  += $$PWD/GVApi.cpp \
            $$PWD/NwReqTracker.cpp \
            $$PWD/AsyncTaskToken.cpp \
            $$PWD/CookieJar.cpp \
            $$PWD/GvXMLParser.cpp \
            $$PWD/MyXmlErrorHandler.cpp \
            $$PWD/GContactsApi.cpp \
            $$PWD/ContactsParser.cpp \
            $$PWD/ContactsXmlHandler.cpp

HEADERS  += $$PWD/api_common.h \
            $$PWD/GVApi.h \
            $$PWD/NwReqTracker.h \
            $$PWD/AsyncTaskToken.h \
            $$PWD/CookieJar.h \
            $$PWD/GvXMLParser.h \
            $$PWD/MyXmlErrorHandler.h \
            $$PWD/GContactsApi.h \
            $$PWD/ContactsParser.h \
            $$PWD/ContactsXmlHandler.h

QT *= network sql xml xmlpatterns script
