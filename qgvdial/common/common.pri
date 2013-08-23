INCLUDEPATH += $$PWD

SOURCES  += $$PWD/main.cpp \
            $$PWD/IMainWindow.cpp \
            $$PWD/CacheDb.cpp \
            $$PWD/CacheDb_p.cpp \
            $$PWD/Lib.cpp \
            $$PWD/ContactsModel.cpp \
            $$PWD/ContactDetailsModel.cpp \
            $$PWD/InboxModel.cpp \
            $$PWD/LibContacts.cpp \
            $$PWD/LibInbox.cpp \
    ../common/LibGvPhones.cpp

HEADERS  += $$PWD/global.h \
            $$PWD/IOsDependent.h \
            $$PWD/IMainWindow.h \
            $$PWD/CacheDb.h \
            $$PWD/CacheDb_p.h \
            $$PWD/Lib.h \
            $$PWD/ContactsModel.h \
            $$PWD/ContactDetailsModel.h \
            $$PWD/InboxModel.h \
            $$PWD/LibContacts.h \
            $$PWD/LibInbox.h \
    ../common/LibGvPhones.h
