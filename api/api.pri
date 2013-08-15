INCLUDEPATH += $$PWD

SOURCES  += $$PWD/GVApi.cpp \
            $$PWD/NwReqTracker.cpp \
            $$PWD/AsyncTaskToken.cpp \
            $$PWD/CookieJar.cpp \
            $$PWD/GvXMLParser.cpp \
            $$PWD/MyXmlErrorHandler.cpp

HEADERS  += $$PWD/GVApi.h \
            $$PWD/NwReqTracker.h \
            $$PWD/AsyncTaskToken.h \
            $$PWD/CookieJar.h \
            $$PWD/GvXMLParser.h \
            $$PWD/MyXmlErrorHandler.h

QT *= network sql xml xmlpatterns script
