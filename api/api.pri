INCLUDEPATH += $$PWD

SOURCES  += $$PWD/GVApi.cpp \
            $$PWD/GVApi_login.cpp \
            $$PWD/NwReqTracker.cpp \
            $$PWD/AsyncTaskToken.cpp \
            $$PWD/CookieJar.cpp \
            $$PWD/MyXmlErrorHandler.cpp \
            $$PWD/GContactsApi.cpp \
            $$PWD/ContactsParser.cpp \
            $$PWD/ContactsXmlHandler.cpp \
            $$PWD/HtmlFieldParser.cpp

HEADERS  += $$PWD/api_common.h \
            $$PWD/GVApi.h \
            $$PWD/GVApi_login.h \
            $$PWD/NwReqTracker.h \
            $$PWD/AsyncTaskToken.h \
            $$PWD/CookieJar.h \
            $$PWD/MyXmlErrorHandler.h \
            $$PWD/GContactsApi.h \
            $$PWD/ContactsParser.h \
            $$PWD/ContactsXmlHandler.h \
            $$PWD/HtmlFieldParser.h

QT *= network xml
lessThan(QT_MAJOR_VERSION, 5) {
QT *= script
}
lessThan(QT_MAJOR_VERSION, 6) {
QT *= xmlpatterns
}

RESOURCES += $$PWD/api.qrc

include($$PWD/../third-party/o2/src/o2.pri)
