/*
 * qgvdial is a cross platform Google Voice Dialer
 * Copyright (C) 2009-2012  Yuvraaj Kelkar
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

class QGVDBusCallApi : public QObject
{
    Q_OBJECT

public:
    QGVDBusCallApi(QObject *parent = NULL);
    ~QGVDBusCallApi();
    bool registerObject();

signals:
    void dialNow (const QString &strNumber);

/////////////////////////////// DBus connections ///////////////////////////////
public Q_SLOTS: // METHODS
    void Call(const QString &strNumber);
};

class QGVDBusTextApi : public QObject
{
    Q_OBJECT

public:
    QGVDBusTextApi(QObject *parent = NULL);
    ~QGVDBusTextApi();
    bool registerObject();

signals:
    void sendText (const QStringList &arrNumbers,
                   const QString     &strData);
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
    QGVDBusSettingsApi(QObject *parent = NULL);
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

#endif//DBUSAPI_H

