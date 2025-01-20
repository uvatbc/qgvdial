INCLUDEPATH += $$PWD/webkit

lessThan(QT_MAJOR_VERSION, 6) {
QT *= webkit
} else {
QT *= webview
}

SOURCES  += $$PWD/webkit/MyWebView.cpp
HEADERS  += $$PWD/webkit/MyWebView.h
