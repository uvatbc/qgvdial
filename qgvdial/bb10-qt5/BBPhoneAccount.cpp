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
#include <QDesktopServices>

#include "BB10PhoneFactory.h"
#include "LibGvPhones.h"
#include "MainWindow.h"

#if !USE_PROCESS
#include <dlfcn.h>
#endif

BBPhoneAccount::BBPhoneAccount(QObject *parent)
: IPhoneAccount(parent)
#if USE_PROCESS
, m_sock(NULL)
#else
, m_hBBPhone(NULL)
, m_phoneCtx(NULL)
#endif
{
#if USE_PROCESS
    QFileInfo fi("app/native/qt4srv");
    if (!QProcess::startDetached (fi.absoluteFilePath ())) {
        Q_WARN("Failed to start process");
    } else {
        QTimer::singleShot (1000, this, SLOT(onProcessStarted()));
    }

    m_TimerLogMessage.setSingleShot (true);
    m_TimerLogMessage.setInterval (1000);
    connect(&m_TimerLogMessage, SIGNAL(timeout()),
            this, SLOT(onLogMessagesTimer()));
#else
    if (NULL == m_hBBPhone) {
        QFileInfo fi("app/native/libbbphone.so");
        m_hBBPhone = dlopen (fi.absoluteFilePath().toLatin1().constData(),
                             RTLD_NOW);
        if (NULL == m_hBBPhone) {
            Q_WARN("Failed to load BB Phone Qt4 library");
            return;
        }
    }
    Q_DEBUG("bbphone lib opened");

    typedef void *(*CreateCtxFn)();
    CreateCtxFn fn = (CreateCtxFn) dlsym(m_hBBPhone,
                                         "createPhoneContext");
    if (NULL == fn) {
        Q_WARN("Failed to get createPhoneContext");
        return;
    }
    Q_DEBUG("Got createPhoneContext");

    m_phoneCtx = fn();
    if (NULL == m_phoneCtx) {
        Q_WARN("Get NULL from createPhoneContext");
    }
#endif
}//BBPhoneAccount::BBPhoneAccount

BBPhoneAccount::~BBPhoneAccount()
{
#if USE_PROCESS
    if (NULL != m_sock) {
        m_sock->write("quit");
        m_sock->waitForBytesWritten (1000);
        delete m_sock;
    }
#else
    if (NULL != m_phoneCtx) {
        typedef void (*DeleteCtxFn)(void *ctx);
        DeleteCtxFn fn = (DeleteCtxFn) dlsym(m_hBBPhone,
                                             "deletePhoneContext");
        if (NULL == fn) {
            Q_WARN("Failed to get deletePhoneContext");
        } else {
            fn(m_phoneCtx);
        }
    }
    if (NULL != m_hBBPhone) {
        dlclose (m_hBBPhone);
        m_hBBPhone = NULL;
    }
#endif
}//BBPhoneAccount::~BBPhoneAccount

#if USE_PROCESS
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
    if (!m_sock->waitForConnected (1000)) {
        Q_WARN("Waiting for a second to connect to server");
        QTimer::singleShot (1000, this, SLOT(onProcessStarted()));
        delete m_sock;
        m_sock = NULL;
        return;
    }

    Q_DEBUG("Socket connected");

    if (!pingPong()) {
        return;
    }

    connect(m_sock, SIGNAL(readyRead()), this, SLOT(onGetNumber()));
    m_sock->write("getNumber");

    m_TimerLogMessage.start ();
}//BBPhoneAccount::onProcessStarted

void
BBPhoneAccount::onGetNumber()
{
    disconnect(m_sock, SIGNAL(readyRead()), this, SLOT(onGetNumber()));

    QByteArray ba = m_sock->readAll ();
    m_number = ba;

    Q_DEBUG(QString("Got number \"%1\". Length of ba = %2").arg (m_number)
            .arg(ba.length()));

    emit numberReady();
}//BBPhoneAccount::onGetNumber

void
BBPhoneAccount::recheckProcess()
{
    if (NULL == m_sock) {
        Q_WARN("NULL socket");
        return;
    }

    m_sock->write("ping");
    m_sock->waitForBytesWritten();
    if (!m_sock->waitForReadyRead ()) {
        // Start the process again
        m_sock->write("quit");
        m_sock->waitForBytesWritten();
        m_sock->deleteLater();
        m_sock = NULL;

        QFileInfo fi("app/native/qt4srv");
        if (!QProcess::startDetached (fi.absoluteFilePath ())) {
            Q_WARN("Failed to start process");
        } else {
            QTimer::singleShot (1000, this, SLOT(onProcessStarted()));
        }
    } else {
        if (m_sock->readAll().startsWith("pong")) {
            m_TimerLogMessage.start ();
        }
    }
}//BBPhoneAccount::recheckProcess

bool
BBPhoneAccount::pingPong()
{
    m_sock->write("ping");
    m_sock->waitForBytesWritten();
    if (!m_sock->waitForReadyRead()) {
        recheckProcess();
        return false;
    }

    QByteArray pong = m_sock->readAll();
    if (!pong.startsWith("pong")) {
        recheckProcess();
        return false;
    }
    if (!pong.contains("first")) {
        Q_DEBUG("We're the second instance!");

        delete m_sock;
        m_sock = NULL;

        qApp->quit();
        return false;
    }
    if (pong.contains("wakeup")) {
        // Wake up! : Hackity hack!
        BB10PhoneFactory *f = (BB10PhoneFactory *) this->parent ();
        LibGvPhones *gvp = (LibGvPhones *) f->parent ();
        MainWindow *win = (MainWindow *) gvp->parent ();

        win->messageReceived ("show");
    }

    return true;
}//BBPhoneAccount::pingPong

void
BBPhoneAccount::onLogMessagesTimer()
{
    m_sock->write("getDebugMessages");
    m_sock->waitForBytesWritten();

    if (!m_sock->waitForReadyRead ()) {
        recheckProcess();
        return;
    }

    QString msgs = m_sock->readAll ();
    if (msgs.length ()) {
        Q_DEBUG(msgs);
    }

    m_TimerLogMessage.start ();
}//BBPhoneAccount::onLogMessagesTimer

#endif

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
    QString dest = task->inParams["destination"].toString();

#if USE_PROCESS
    if (NULL == m_sock) {
        Q_WARN("BB phone library is not initialized");
        task->status = ATTS_FAILURE;
        task->emitCompleted();
        return true;
    }

    //dest = "initiateCellularCall" + dest;
    //m_sock->write(dest.toLatin1 ());

    QDesktopServices::openUrl(QUrl("tel:" + dest));
#else
    typedef void (*InitiateCallFn)(void *ctx, const char *dest);
    InitiateCallFn fn = (InitiateCallFn) dlsym(m_hBBPhone,
                                               "initiateCellularCall");
    if (NULL == fn) {
        Q_WARN("Failed to get initiateCellularCall");
        task->status = ATTS_FAILURE;
        task->emitCompleted();
        return true;
    } else {
        fn(m_phoneCtx, dest.toLatin1().constData());
    }
#endif

    Q_DEBUG(QString("Call initiated to dest: %1").arg(dest));

    //TODO: Do this in the slot for the completion of the phone call
    task->status = ATTS_SUCCESS;
    task->emitCompleted();
    return true;
}//BBPhoneAccount::initiateCall

QString
BBPhoneAccount::getNumber()
{
#if USE_PROCESS
    return m_number;
#else
    if (NULL == m_phoneCtx) {
        Q_WARN("BB phone library is not initialized");
        return QString();
    }

    typedef const char *(*GetNumFn)(void *ctx);
    GetNumFn fn = (GetNumFn) dlsym(m_hBBPhone, "getNumber");
    const char *bbrv = fn(m_phoneCtx);

    QString rv;
    rv += bbrv;

    return rv;
#endif
}//BBPhoneAccount::getNumber
