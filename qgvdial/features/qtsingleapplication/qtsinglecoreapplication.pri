INCLUDEPATH	 += $$PWD/inc
DEPENDPATH   += $$PWD/inc

HEADERS		 += $$PWD/inc/qtsinglecoreapplication.h     \
                $$PWD/inc/qtlockedfile.h                \
                $$PWD/inc/qtlocalpeer.h

SOURCES      += $$PWD/src/qtsinglecoreapplication.cpp   \
                $$PWD/src/qtlockedfile_win.cpp          \
                $$PWD/src/qtlockedfile_unix.cpp         \
                $$PWD/src/qtlockedfile.cpp              \
                $$PWD/src/qtlocalpeer.cpp

QT *= network
