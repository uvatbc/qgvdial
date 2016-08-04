/*
 * qgvdial is a cross platform Google Voice Dialer
 * Copyright (C) 2009-2016  Yuvraaj Kelkar
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Contact: yuvraaj@gmail.com
 */

#ifndef DBUSAPI_H
#define DBUSAPI_H

#include "global.h"

class IMainWindow;

class QGVDBusCallApi : public QObject
{
    Q_OBJECT

public:
    QGVDBusCallApi(IMainWindow *parent = NULL);
    ~QGVDBusCallApi();
    bool registerObject();

/////////////////////////////// DBus connections ///////////////////////////////
public Q_SLOTS: // METHODS
    void Call(const QString &strNumber);
};

class QGVDBusTextApi : public QObject
{
    Q_OBJECT

public:
    QGVDBusTextApi(IMainWindow *parent = NULL);
    ~QGVDBusTextApi();
    bool registerObject();

signals:
    void sendTextWithoutData (const QStringList &arrNumbers);

/////////////////////////////// DBus connections ///////////////////////////////
public Q_SLOTS: // METHODS
    void Text(const QStringList &arrNumbers, const QString &strData);
    void TextWithoutData(const QStringList &arrNumbers);
    QStringList getTextsByContact(const QString &strContact);
    QStringList getTextsByDate(const QString &strStart, const QString &strEnd);
};

class QGVDBusSettingsApi : public QObject
{
    Q_OBJECT

public:
    QGVDBusSettingsApi(IMainWindow *parent = NULL);
    ~QGVDBusSettingsApi();
    bool registerObject();

signals:
    bool phoneIndexChange(int index);

public Q_SLOTS:
    void onPhoneChanges(const QStringList &phones, int index);

private:
    QStringList m_phones;
    int m_phoneIndex;

/////////////////////////////// DBus connections ///////////////////////////////
public Q_SLOTS: // METHODS
    int GetCurrentPhone();
    QStringList GetPhoneNames();
    bool SetCurrentPhone(int index);
Q_SIGNALS: // SIGNALS
    void CallbacksChanged();
};

class QGVDBusUiApi : public QObject
{
    Q_OBJECT

public:
    QGVDBusUiApi(IMainWindow *parent = NULL);
    ~QGVDBusUiApi();
    bool registerObject();

signals:
    void sigOpenInbox();
    void sigOpenContacts();
    void sigQuit();
    void sigRefresh();
    void sigShow();

/////////////////////////////// DBus connections ///////////////////////////////
public: // PROPERTIES
public Q_SLOTS: // METHODS
    void OpenContacts();
    void OpenInbox();
    void Quit();
    void Refresh();
    void Show();
Q_SIGNALS: // SIGNALS
};

#endif//DBUSAPI_H

