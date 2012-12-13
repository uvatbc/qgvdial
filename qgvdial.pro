TEMPLATE = subdirs
SUBDIRS  = src

#maemo5: SUBDIRS += qgv-tp qgv-util
maemo5: SUBDIRS += qgv-tp
unix:!symbian:!maemo5 {
    SUBDIRS *= qgv-tp
}

