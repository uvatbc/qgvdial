#ifndef WEBWIDGET_H
#define WEBWIDGET_H

#include "global.h"
#include <QtWebKit>

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class WebWidget : public QDeclarativeItem
{
    Q_OBJECT

public:
    explicit WebWidget (QDeclarativeItem *parent = 0);
    ~WebWidget();

private:
    void keyPressEvent (QKeyEvent *event);

private:
    QWebView *wv;
    QGraphicsProxyWidget *proxy;
};

#endif // WEBWIDGET_H
