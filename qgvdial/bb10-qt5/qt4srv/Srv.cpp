/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2015  Yuvraaj Kelkar

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
, m_srv(this)
, m_single(NULL)
{
    connect(&m_phone,
            SIGNAL(onCallCommandResponseReceived(bb::system::phone::CallCommandResponse)),
            this,
            SLOT(onCallCommandResponseReceived(bb::system::phone::CallCommandResponse)));

    int count = 0;
    while (!m_srv.listen ("qgvdial") && (++count < 5)) {
        qWarning ("Server is already listening");

        QLocalSocket sock;
        sock.connectToServer ("qgvdial");
        sock.waitForConnected (10*1000);

        sock.write("ping");
        qDebug("Wrote ping");
        sock.waitForBytesWritten (10 * 1000);
        if (sock.waitForReadyRead (10 * 1000)) {
            QByteArray resp = sock.readAll ();
            qDebug() << QString("Ping response = \"%1\"").arg(QString(resp));
            if (resp.startsWith ("pong")) {
                qDebug("Pong! time to leave");
                qApp->quit ();
                exit(0);
                return;
            }
        }

        sock.write ("quit");
        qDebug ("Wrote quit");
        sock.waitForBytesWritten (10 * 1000);

        m_srv.removeServer ("qgvdial");
    }

    if (count >= 5) {
        qDebug ("Self quit");
        qApp->quit ();
        exit(-1);
        return;
    }

    QObject::connect(&m_srv, SIGNAL(newConnection()),
                     this, SLOT(onNewConnection()));
}//MainObject::MainObject

MainObject::~MainObject()
{
    m_srv.close ();
    m_srv.removeServer ("qgvdial");
}//MainObject::~MainObject

void
MainObject::onNewConnection()
{
    QLocalSocket *c;

    while ((c = m_srv.nextPendingConnection ())) {
        if (NULL == m_single)  {
            m_single = c;
        }
        
        connect(c, SIGNAL(stateChanged(QLocalSocket::LocalSocketState)),
                this, SLOT(onStateChanged(QLocalSocket::LocalSocketState)));
        connect(c, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

        saveMessage("SRV: Got one connection!!");
    }
}//MainObject::onNewConnection

void
MainObject::onStateChanged(QLocalSocket::LocalSocketState socketState)
{
    QLocalSocket *c = (QLocalSocket *) QObject::sender ();
    if (QLocalSocket::ClosingState == socketState) {
        c->deleteLater ();
        
        if (c == m_single) {
            m_single = NULL;
            saveMessage("SRV: Clearing out main connection!!");
        } else {
            saveMessage("SRV: Clearing out one connection!!");
        }
    }
}//MainObject::onStateChanged

void
MainObject::onReadyRead()
{
    QLocalSocket *c = (QLocalSocket *) QObject::sender ();
    QByteArray data = c->readAll ();

    if (data.startsWith ("quit")) {
        saveMessage("SRV: Quitting now!");
        qApp->quit ();
        exit (0);
        return;
    }

    if (data.startsWith ("getNumber")) {
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

        saveMessage(QString("SRV: getNumber: Returning \"%1\"").arg(number));
        return;
    }

    if (data.startsWith ("initiateCellularCall")) {
        QString dest = data.mid(sizeof("initiateCellularCall") - 1);
        QString msg = QString("SRV: initiateCellularCall: %1").arg(dest);

        m_phone.initiateCellularCall (dest);
        saveMessage(msg);
        return;
    }

    if (data.startsWith ("getDebugMessages")) {
        QString msgs = m_msgs.join ("\n");
        c->write(msgs.toLatin1().constData(), msgs.length()+1);
        m_msgs.clear ();
        return;
    }

    if (data.startsWith ("ping")) {
        QByteArray msg = "pong";
        if (c == m_single) {
            msg += "first";
            if (m_wakeupFirst) {
                msg += "wakeup";
            }
        }
        
        c->write (msg);
        return;
    }

    saveMessage (QString("Invalid data: \"%1\"").arg (QString(data)));
}//MainObject::onReadyRead

void
MainObject::saveMessage(const QString &msg)
{
    QDateTime dt = QDateTime::currentDateTime ();
    m_msgs.append(QString("%1: %2").arg (dt.toString (Qt::ISODate), msg));
    qDebug() << msg;
}//MainObject::saveMessage

void
MainObject::onCallCommandResponseReceived(const bb::system::phone::CallCommandResponse &commandResponse)
{
    QString err = commandResponse.error();
    if (!err.isEmpty()) {
        saveMessage(err);
    }
}//MainObject::onCallCommandResponseReceived
