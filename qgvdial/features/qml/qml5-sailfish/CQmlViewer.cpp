/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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

#include "CQmlViewer.h"
#include <sailfishapp.h>

CQmlViewer *
createQmlViewer()
{
    return new CQmlViewer;
}//createQmlViewer

CQmlViewer::CQmlViewer()
: m_view(SailfishApp::createView())
{
    if (NULL == m_view) {
        Q_WARN("Failed to create view");
    }

    connect(m_view, SIGNAL(statusChanged(QQuickView::Status)),
            this, SLOT(onDeclStatusChanged(QQuickView::Status)));
}//CQmlViewer::CQmlViewer

void
CQmlViewer::onDeclStatusChanged(QQuickView::Status status)
{
    if (QQuickView::Ready == status) {
        emit viewerStatusChanged (true);
        return;
    }

    if (QQuickView::Error != status) {
        return;
    }

    Q_WARN(QString("status = %1").arg (status));
    emit viewerStatusChanged (false);
}//CQmlViewer::onDeclStatusChanged

bool
CQmlViewer::connectToChangeNotify(QObject *item, const QString &propName,
                                  QObject *receiver, const char *slotName)
{
    bool rv = false;
    do {
        const QMetaObject *metaObject = item->metaObject ();
        if (NULL == metaObject) {
            Q_WARN("NULL metaObject");
            break;
        }

        QMetaProperty metaProp;
        for (int i = 0; i < metaObject->propertyCount (); i++) {
            metaProp = metaObject->property (i);
            if (metaProp.name () == propName) {
                rv = true;
                break;
            }
        }

        if (!rv) {
            Q_WARN(QString("Couldn't find property named %1").arg(propName));
            break;
        }
        rv = false;

        if (!metaProp.hasNotifySignal ()) {
            Q_WARN(QString("Property %1 does not have a notify signal")
                   .arg(propName));
            break;
        }

        QString signalName = metaProp.notifySignal().methodSignature();
        signalName = "2" + signalName;

        Q_DEBUG(QString("Connect %1 to %2").arg(signalName).arg(slotName));

        rv =
        connect(item, signalName.toLatin1().constData(), receiver, slotName);
    } while(0);

    return (rv);
}//CQmlViewer::connectToChangeNotify
