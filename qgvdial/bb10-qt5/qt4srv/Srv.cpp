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

#include "Srv.h"

MainObject::MainObject(QObject *parent)
: QObject(parent)
{
}//MainObject::MainObject

void
MainObject::onNewConnection()
{
    QLocalServer *s = (QLocalServer *) QObject::sender ();
    QLocalSocket *c;

    while ((c = s->nextPendingConnection ())) {
        connect(c, SIGNAL(stateChanged(QLocalSocket::LocalSocketState)),
                this, SLOT(onStateChanged(QLocalSocket::LocalSocketState)));
        connect(c, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

        qDebug("SRV: Got one connection!!");
    }
}//MainObject::onNewConnection

void
MainObject::onStateChanged(QLocalSocket::LocalSocketState socketState)
{
    QLocalSocket *c = (QLocalSocket *) QObject::sender ();
    if (QLocalSocket::ClosingState == socketState) {
        c->deleteLater ();
        qDebug("SRV: Clearing out one connection!!");
    }
}//MainObject::onStateChanged

void
MainObject::onReadyRead()
{
    QLocalSocket *c = (QLocalSocket *) QObject::sender ();
    QByteArray data = c->readAll ();

    if (data.startsWith ("quit")) {
        qDebug("SRV: Quitting now!");
        qApp->quit ();
        exit (0);
        return;
    }

    if (data.startsWith ("getNumber")) {
        qDebug("SRV: getNumber!");

        QString number;
#ifndef Q_WS_SIMULATOR
        QMap <QString, bb::system::phone::Line> l = m_phone.lines();
        foreach (QString key, l.keys()) {
            if (l[key].type() == bb::system::phone::LineType::Cellular) {
                number = l[key].address();
                break;
            }
        }
#endif
        c->write(number.toLatin1().constData(), number.length()+1);
        return;
    }

    if (data.startsWith ("initiateCellularCall")) {
        qDebug("SRV: initiateCellularCall!");

        data = data.mid(sizeof("initiateCellularCall") - 1);

        m_phone.initiateCellularCall (QString(data));
        return;
    }
}//MainObject::onReadyRead
