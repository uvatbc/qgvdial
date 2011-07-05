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
#include "Singletons.h"

QGVDbusServerHelper::QGVDbusServerHelper (QGVDbusSettingsServer *s,
                                          QObject *parent)
: QObject (parent)
, settingsServer (s)
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

void
QGVDbusServerHelper::onPhoneChanges(const QStringList &phones, int index)
{
    if (NULL != settingsServer) {
        settingsServer->callbacks = phones;
        settingsServer->phoneIndex = index;
        settingsServer->emitCallbacksChanged ();
    }
}//QGVDbusServerHelper::onPhoneChanges

void
QGVDbusServerHelper::emitPhoneIndexChange(int index)
{
    emit phoneIndexChange (index);
}//QGVDbusServerHelper::emitPhoneIndexChange

QGVDbusCallServer::QGVDbusCallServer (QObject *parent)
: QDBusAbstractAdaptor(parent)
, helper (NULL, this)
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
, helper (NULL, this)
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

QStringList
QGVDbusTextServer::getTextsByDate(const QString &strStart, const QString &strEnd)
{
    QStringList rv;
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    QDateTime dtStart = QDateTime::fromString(strStart, Qt::ISODate);
    QDateTime dtEnd = QDateTime::fromString(strEnd, Qt::ISODate);

    do { // Begin cleanup block (not a loop)
        if (!dtStart.isValid () || !dtEnd.isValid ()) {
            qWarning ("getTextsByDate: Invalid dates");
            break;
        }

        // If reversed, fix it
        if (dtStart > dtEnd) {
            QDateTime dtTemp = dtEnd;
            dtEnd = dtStart;
            dtStart = dtTemp;
        }

        // Send request
        rv = dbMain.getTextsByDate (dtStart, dtEnd);
    } while(0); // End cleanup block (not a loop)

    return rv;
}//QGVDbusTextServer::getTextsByDate

QStringList
QGVDbusTextServer::getTextsByContact(const QString &strContact)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    return dbMain.getTextsByContact(strContact);
}//QGVDbusTextServer::getTextsByContact

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

QGVDbusSettingsServer::QGVDbusSettingsServer(QObject *parent)
: QDBusAbstractAdaptor (parent)
, helper (this, this)
{
}//QGVDbusSettingsServer::QGVDbusSettingsServer

void
QGVDbusSettingsServer::addSettingsReceiver (QObject *r1, const char *m1,
                                            QObject *r2, const char *m2)
{
    connect (&helper, SIGNAL (phoneIndexChange(int )), r1, m1);
    connect (r2, m2, &helper, SLOT(onPhoneChanges(const QStringList &,int)));
}//QGVDbusSettingsServer::addSettingsReceiver

void
QGVDbusSettingsServer::emitCallbacksChanged ()
{
    emit CallbacksChanged();
}//QGVDbusSettingsServer::emitCallbacksChanged

QStringList
QGVDbusSettingsServer::GetPhoneNames ()
{
    qDebug ("DBus request to get phone names");
    return callbacks;
}//QGVDbusSettingsServer::GetPhoneNames

int
QGVDbusSettingsServer::GetCurrentPhone ()
{
    qDebug ("DBus request to get current phone");
    return phoneIndex;
}//QGVDbusSettingsServer::GetCurrentPhone

void
QGVDbusSettingsServer::SetCurrentPhone (int index)
{
    qDebug ("DBus request to set current phone");
    helper.emitPhoneIndexChange (index);
}//QGVDbusSettingsServer::SetCurrentPhone
