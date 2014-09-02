/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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
#include <QtDeclarative>

CQmlViewer *
createQmlViewer()
{
    return new CQmlViewer;
}

CQmlViewer::CQmlViewer()
{
    connect(this, SIGNAL(statusChanged(QDeclarativeView::Status)),
            this, SLOT(onDeclStatusChanged(QDeclarativeView::Status)));
}//CQmlViewer::CQmlViewer

void
CQmlViewer::onDeclStatusChanged(QDeclarativeView::Status status)
{
    if (QDeclarativeView::Ready == status) {
        emit viewerStatusChanged (true);
        return;
    }

    if (QDeclarativeView::Error != status) {
        return;
    }

    Q_WARN(QString("status = %1").arg (status));
    emit viewerStatusChanged (false);
}//CQmlViewer::onDeclStatusChanged

QDeclarativeContext *
CQmlViewer::rootContext() const
{
    return this->engine()->rootContext();
}//CQmlViewer::rootContext
