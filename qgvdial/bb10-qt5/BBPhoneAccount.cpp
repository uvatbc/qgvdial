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

#include "BBPhoneAccount.h"

BBPhoneAccount::BBPhoneAccount(QObject *parent)
: IPhoneAccount(parent)
, m_sock(NULL)
{
    QFileInfo fi("app/native/qt4srv");
    if (!QProcess::startDetached (fi.absoluteFilePath ())) {
        Q_WARN("Failed to start process");
    } else {
        QTimer::singleShot (1000, this, SLOT(onProcessStarted()));
    }
}//BBPhoneAccount::BBPhoneAccount

BBPhoneAccount::~BBPhoneAccount()
{
    if (NULL != m_sock) {
        m_sock->write("quit");
        m_sock->waitForBytesWritten (1000);
        delete m_sock;
    }
}//BBPhoneAccount::~BBPhoneAccount

void
BBPhoneAccount::onProcessStarted()
{
    Q_DEBUG("Process started!");

    m_sock = new QLocalSocket(this);
    if (NULL == m_sock) {
        Q_WARN("Failed to allocate local socket");
        return;
    }

    m_sock->connectToServer ("qgvdial");
    if (!m_sock->waitForConnected (500)) {
        Q_WARN("Waiting for a second to connect to server");
        QTimer::singleShot (1000, this, SLOT(onProcessStarted()));
        delete m_sock;
        m_sock = NULL;
        return;
    }

    Q_DEBUG("Socket connected");

    connect(m_sock, SIGNAL(readyRead()), this, SLOT(onGetNumber()));
    m_sock->write("getNumber");
}//BBPhoneAccount::onProcessStarted

void
BBPhoneAccount::onGetNumber()
{
    disconnect(m_sock, SIGNAL(readyRead()), this, SLOT(onGetNumber()));
    m_number = m_sock->readAll ();

    Q_DEBUG(QString("Got numnber %1").arg (m_number));
}//BBPhoneAccount::onGetNumber

QString
BBPhoneAccount::id()
{
    return "ring";
}//BBPhoneAccount::id

QString
BBPhoneAccount::name()
{
    return "This phone";
}//BBPhoneAccount::name

bool
BBPhoneAccount::initiateCall(AsyncTaskToken *task)
{
    if (!task->inParams.contains("destination")) {
        Q_WARN("Destination not given!");
        task->status = ATTS_INVALID_PARAMS;
        task->emitCompleted();
        return true;
    }

    if (NULL == m_sock) {
        Q_WARN("BB phone library is not initialized");
        task->status = ATTS_FAILURE;
        task->emitCompleted();
        return true;
    }

    QString dest = task->inParams["destination"].toString();

    dest = "initiateCellularCall" + dest;
    m_sock->write(dest.toLatin1 ());

    //TODO: Do this in the slot for the completion of the phone call
    task->status = ATTS_SUCCESS;
    task->emitCompleted();
    return true;
}//BBPhoneAccount::initiateCall

QString
BBPhoneAccount::getNumber()
{
    return m_number;
}//BBPhoneAccount::getNumber
