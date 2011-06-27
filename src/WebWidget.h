/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

#ifndef WEBWIDGET_H
#define WEBWIDGET_H

#include "global.h"
#include <QtWebKit>
#include <QtDeclarative>

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
    void geometryChanged (const QRectF &newG, const QRectF &oldG);

private:
    QWebView *wv;
    QGraphicsProxyWidget *proxy;
};

#endif // WEBWIDGET_H
