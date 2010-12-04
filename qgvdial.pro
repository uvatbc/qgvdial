TEMPLATE = subdirs
SUBDIRS  = src

maemo5: SUBDIRS += qgv-tp qgv-util
unix:!symbian:!maemo5 {
    SUBDIRS += qgv-tp qgv-util
}

