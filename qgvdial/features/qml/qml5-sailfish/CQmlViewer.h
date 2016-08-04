/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016  Yuvraaj Kelkar

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

#ifndef _CQMLVIEWER_
#define _CQMLVIEWER_

#include "global.h"
#include <QtQuick/QtQuick>

class QmlApplicationViewer {
public:
    enum ScreenOrientation {
        ScreenOrientationAuto
    };
};

class CQmlViewer : public QObject
{
    Q_OBJECT

public:
    CQmlViewer();

    inline void setMainQmlFile(const QString &qmlFile) {
        m_view->setSource(QUrl(qmlFile));
    }

    // Stupid hack to shut up the warnings
    inline void setOrientation(QmlApplicationViewer::ScreenOrientation /*o*/) {}

    // Stub off these show functions.
    inline void show() { m_view->show(); }
    inline void showExpanded() { show(); }

    // Root object and context need to be provided
    inline QQuickItem *rootObject() { return m_view->rootObject(); }
    inline QQmlContext *rootContext() const { return m_view->rootContext (); }

    bool connectToChangeNotify(QObject *item, const QString &propName,
                               QObject *receiver, const char *slotName);

signals:
    void viewerStatusChanged(bool ready);

private slots:
    void onDeclStatusChanged(QQuickView::Status status);

private:
    QQuickView *m_view;
};

CQmlViewer *createQmlViewer();

#endif//_CQMLVIEWER_

