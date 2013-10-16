/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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

#include "DBusApi.h"
#include "gen/api_adapter.h"
#include "IMainWindow.h"

QGVDBusCallApi::QGVDBusCallApi(IMainWindow *parent)
: QObject(parent)
{
}//QGVDBusCallApi::QGVDBusCallApi

QGVDBusCallApi::~QGVDBusCallApi()
{
}//QGVDBusCallApi::~QGVDBusCallApi

bool
QGVDBusCallApi::registerObject()
{
    CallServerAdaptor *obj = new CallServerAdaptor(this);
    if (NULL == obj) {
        return false;
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerObject ("/org/QGVDial/CallServer", this)) {
        delete obj;
        return false;
    }

    return true;
}//QGVDBusCallApi::registerObject

void
QGVDBusCallApi::Call(const QString &strNumber)
{
    IMainWindow *win = (IMainWindow *)parent ();
    Q_DEBUG(QString("Make a call to %1").arg (strNumber));
    win->onUserCall (strNumber);
}//QGVDBusCallApi::Call

QGVDBusTextApi::QGVDBusTextApi(IMainWindow *parent)
: QObject(parent)
{
}//QGVDBusTextApi::QGVDBusTextApi

QGVDBusTextApi::~QGVDBusTextApi()
{
}//QGVDBusTextApi::~QGVDBusTextApi

bool
QGVDBusTextApi::registerObject()
{
    TextServerAdaptor *obj = new TextServerAdaptor(this);
    if (NULL == obj) {
        return false;
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerObject ("/org/QGVDial/TextServer", this)) {
        delete obj;
        return false;
    }

    return true;
}//QGVDBusTextApi::registerObject

void
QGVDBusTextApi::Text(const QStringList &arrNumbers, const QString &strData)
{
    Q_DEBUG(QString("Send text \"%1\" to (%2)")
            .arg (strData)
            .arg (arrNumbers.join (", ")));

    IMainWindow *win = (IMainWindow *) parent ();
    win->onUserSendSMS (arrNumbers, strData);
}//QGVDBusTextApi::Text

void
QGVDBusTextApi::TextWithoutData(const QStringList &arrNumbers)
{
    Q_DEBUG(QString("Send text without data to (%1)")
            .arg (arrNumbers.join (", ")));
    emit sendTextWithoutData (arrNumbers);
}//QGVDBusTextApi::TextWithoutData

QStringList
QGVDBusTextApi::getTextsByContact(const QString &strContact)
{
    IMainWindow *win = (IMainWindow *) parent ();
    QStringList rv = win->getTextsByContact(strContact);

    Q_DEBUG(QString("Got dbus request to get texts from contact matching "
                    "\"%1\". Returning %2 texts")
            .arg(strContact).arg (rv.count ()));

    return rv;
}//QGVDBusTextApi::getTextsByContact

QStringList
QGVDBusTextApi::getTextsByDate(const QString &strStart, const QString &strEnd)
{
    IMainWindow *win = (IMainWindow *) parent ();
    QStringList rv;

    QDateTime dtStart = QDateTime::fromString(strStart, Qt::ISODate);
    QDateTime dtEnd = QDateTime::fromString(strEnd, Qt::ISODate);

    do { // Begin cleanup block (not a loop)
        if (!dtStart.isValid () || !dtEnd.isValid ()) {
            Q_WARN("Invalid dates");
            break;
        }

        // If reversed, fix it
        if (dtStart > dtEnd) {
            QDateTime dtTemp = dtEnd;
            dtEnd = dtStart;
            dtStart = dtTemp;
        }

        // Send request
        rv = win->getTextsByDate (dtStart, dtEnd);

        Q_DEBUG(QString("Got dbus request to get texts between %1 and %2. "
                        "Returning %3 texts")
                .arg(strStart, strEnd).arg (rv.count ()));
    } while(0); // End cleanup block (not a loop)

    return rv;
}//QGVDBusTextApi::getTextsByDate

QGVDBusSettingsApi::QGVDBusSettingsApi(IMainWindow *parent)
: QObject(parent)
, m_phoneIndex(-1)
{
}//QGVDBusSettingsApi::QGVDBusSettingsApi

QGVDBusSettingsApi::~QGVDBusSettingsApi()
{
}//QGVDBusSettingsApi::~QGVDBusSettingsApi

bool
QGVDBusSettingsApi::registerObject()
{
    SettingsServerAdaptor *obj = new SettingsServerAdaptor(this);
    if (NULL == obj) {
        return false;
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerObject ("/org/QGVDial/SettingsServer", this)) {
        delete obj;
        return false;
    }

    return true;
}//QGVDBusSettingsApi::registerObject

int
QGVDBusSettingsApi::GetCurrentPhone()
{
    Q_DEBUG("DBus request to get current phone index");
    return m_phoneIndex;
}//QGVDBusSettingsApi::GetCurrentPhone

QStringList
QGVDBusSettingsApi::GetPhoneNames()
{
    Q_DEBUG("DBus request to get phone names");
    return m_phones;
}//QGVDBusSettingsApi::GetPhoneNames

bool
QGVDBusSettingsApi::SetCurrentPhone(int index)
{
    return (emit phoneIndexChange(index));
}//QGVDBusSettingsApi::SetCurrentPhone

void
QGVDBusSettingsApi::onPhoneChanges(const QStringList &phones, int index)
{
    m_phones = phones;
    m_phoneIndex = index;
    emit CallbacksChanged();
}//QGVDBusSettingsApi::onPhoneChanges

QGVDBusUiApi::QGVDBusUiApi(IMainWindow *parent)
: QObject(parent)
{
}//QGVDBusUiApi::QGVDBusUiApi

QGVDBusUiApi::~QGVDBusUiApi()
{
}//QGVDBusUiApi::~QGVDBusUiApi

bool
QGVDBusUiApi::registerObject()
{
    UIServerAdaptor *obj = new UIServerAdaptor(this);
    if (NULL == obj) {
        return false;
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerObject ("/org/QGVDial/UIServer", this)) {
        delete obj;
        return false;
    }

    return true;
}//QGVDBusUiApi::registerObject

void
QGVDBusUiApi::OpenContacts()
{
    emit sigOpenContacts();
}//QGVDBusUiApi::OpenContacts

void
QGVDBusUiApi::OpenInbox()
{
    emit sigOpenInbox();
}//QGVDBusUiApi::OpenInbox

void
QGVDBusUiApi::Quit()
{
    emit sigQuit();
}//QGVDBusUiApi::Quit

void
QGVDBusUiApi::Refresh()
{
    emit sigRefresh();
}//QGVDBusUiApi::Refresh
