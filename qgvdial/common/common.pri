INCLUDEPATH *= $$PWD
PRECOMPILED_HEADER = $$PWD/global.h
QT *= sql

##############################################################
# YOU MUST ADD YOUR OWN MOBILITY OR PHONON LIBRARY: Either:
#CONFIG *= mobility
#MOBILITY *= multimedia
# *** OR ***
#QT *= phonon
##############################################################

SOURCES  += $$PWD/main.cpp \
            $$PWD/IMainWindow.cpp \
            $$PWD/CacheDb.cpp \
            $$PWD/CacheDb_p.cpp \
            $$PWD/Lib.cpp \
            $$PWD/ContactsModel.cpp \
            $$PWD/ContactNumbersModel.cpp \
            $$PWD/InboxModel.cpp \
            $$PWD/LibContacts.cpp \
            $$PWD/LibInbox.cpp \
            $$PWD/LibGvPhones.cpp \
            $$PWD/IObserverFactory.cpp \
            $$PWD/GVNumModel.cpp \
            $$PWD/LogUploader.cpp \
            $$PWD/LibVmail.cpp \
            $$PWD/O2ContactsStore.cpp

HEADERS  += $$PWD/global.h \
            $$PWD/IOsDependent.h \
            $$PWD/IMainWindow.h \
            $$PWD/CacheDb.h \
            $$PWD/CacheDb_p.h \
            $$PWD/Lib.h \
            $$PWD/ContactsModel.h \
            $$PWD/ContactNumbersModel.h \
            $$PWD/InboxModel.h \
            $$PWD/LibContacts.h \
            $$PWD/LibInbox.h \
            $$PWD/LibGvPhones.h \
            $$PWD/IObserverFactory.h \
            $$PWD/IObserver.h \
            $$PWD/GVNumModel.h \
            $$PWD/LogUploader.h \
            $$PWD/LibVmail.h \
            $$PWD/O2ContactsStore.h

include($$PWD/phone-account.pri)
include($$PWD/../../api/api.pri)
