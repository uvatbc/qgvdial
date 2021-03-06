QT *= network
lessThan(QT_MAJOR_VERSION, 5) {
QT *= script
}

INCLUDEPATH += $$PWD
SOURCES += \
    $$PWD/o1.cpp \
    $$PWD/o1requestor.cpp \
    $$PWD/o2.cpp \
    $$PWD/o2reply.cpp \
    $$PWD/o2replyserver.cpp \
    $$PWD/o2requestor.cpp \
    $$PWD/simplecrypt.cpp \
    $$PWD/o2settingsstore.cpp

HEADERS += \
    $$PWD/o1.h \
    $$PWD/o1requestor.h \
    $$PWD/o2.h \
    $$PWD/o2reply.h \
    $$PWD/o2replyserver.h \
    $$PWD/o2requestor.h \
    $$PWD/simplecrypt.h \
    $$PWD/o2globals.h \
    $$PWD/o2abstractstore.h \
    $$PWD/o2settingsstore.h

#SOURCES  += $$PWD/o2facebook.cpp \
#            $$PWD/o1dropbox.h \
#            $$PWD/o1flickr.h \
#            $$PWD/o2gft.h \
#            $$PWD/o2skydrive.cpp \
#            $$PWD/oxtwitter.cpp
#HEADERS  += $$PWD/o2facebook.h \
#            $$PWD/o2gft.cpp \
#            $$PWD/o2skydrive.h \
#            $$PWD/oxtwitter.h \
#            $$PWD/o1twitter.h \
