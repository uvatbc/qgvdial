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

#include "QGVDbusServer.h"

QGVDbusServerHelper::QGVDbusServerHelper (QObject *parent)
: QObject (parent)
{
}//QGVDbusServerHelper::QGVDbusServerHelper

void
QGVDbusServerHelper::emitDialNow (const QString &strNumber)
{
    emit dialNow (strNumber);
}//QGVDbusServerHelper::emitDialNow

void
QGVDbusServerHelper::emitText (const QStringList &arrNumbers,
                               const QString     &strData)
{
    QString msg = QString("Send text \"%1\" to (%2)")
                  .arg (strData)
                  .arg (arrNumbers.join (", "));
    qDebug () << msg;
    emit sendText (arrNumbers, strData);
}//QGVDbusServerHelper::emitText

void
QGVDbusServerHelper::emitTextWithoutData (const QStringList &arrNumbers)
{
    QString msg = QString("Send text without data to (%1)")
                  .arg (arrNumbers.join (", "));
    qDebug () << msg;
    emit sendTextWithoutData (arrNumbers);
}//QGVDbusServerHelper::emitTextWithoutData

QGVDbusCallServer::QGVDbusCallServer (QObject *parent)
: QDBusAbstractAdaptor(parent)
, helper (this)
{
}//QGVDbusCallServer::QGVDbusServer

void
QGVDbusCallServer::Call (const QString &strNumber)
{
    // Make a call
    helper.emitDialNow (strNumber);
}//QGVDbusCallServer::Call

void
QGVDbusCallServer::addCallReceiver (QObject *receiver, const char *method)
{
    QObject::connect (&helper, SIGNAL (dialNow (const QString &)),
                      receiver, method);
}//QGVDbusCallServer::addCallReceiver

void
QGVDbusCallServer::delCallReceiver (QObject *receiver, const char *method)
{
    QObject::disconnect (&helper, SIGNAL (dialNow (const QString &)),
                          receiver, method);
}//QGVDbusCallServer::delCallReceiver

QGVDbusTextServer::QGVDbusTextServer (QObject *parent)
: QDBusAbstractAdaptor(parent)
, helper (this)
{
}//QGVDbusTextServer::QGVDbusServer

void
QGVDbusTextServer::Text (const QStringList &arrNumbers,
                         const QString     &strData)
{
    // Send a text
    helper.emitText (arrNumbers, strData);
}//QGVDbusTextServer::Text

void
QGVDbusTextServer::TextWithoutData (const QStringList &arrNumbers)
{
    // Signal that a text is to be sent to this list of numbers
    helper.emitTextWithoutData (arrNumbers);
}//QGVDbusTextServer::TextWithoutData

void
QGVDbusTextServer::addTextReceivers (QObject *r1, const char *m1,
                                     QObject *r2, const char *m2)
{
    QObject::connect (
        &helper, SIGNAL (sendText (const QStringList &, const QString &)),
        r1, m1);
    QObject::connect (
        &helper, SIGNAL (sendTextWithoutData (const QStringList &)),
        r2, m2);
}//QGVDbusTextServer::addTextReceiver

void
QGVDbusTextServer::delTextReceivers (QObject *r1, const char *m1,
                                     QObject *r2, const char *m2)
{
    QObject::disconnect (
        &helper, SIGNAL (sendText (const QStringList &, const QString &)),
        r1, m1);
    QObject::disconnect (
        &helper, SIGNAL (sendTextWithoutData (const QStringList &)),
        r2, m2);
}//QGVDbusTextServer::delTextReceiver
