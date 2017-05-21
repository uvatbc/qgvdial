/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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

#ifndef LIBGVPHONES_H
#define LIBGVPHONES_H

#include <QObject>
#include "global.h"

class IMainWindow;
class GVNumModel;

class IPhoneAccount;
class IPhoneAccountFactory;

class LibGvPhones : public QObject
{
    Q_OBJECT
public:
    explicit LibGvPhones(IMainWindow *parent);
    ~LibGvPhones();

    bool refresh();
    bool refreshOutgoing();

public slots:
    bool onUserSelectPhone(int index);
    bool onUserSelectPhone(QString id);

    bool onUserUpdateCiNumber(int index);
    bool onUserUpdateCiNumber(QString id);

    bool linkCiToNumber(QString ciId, QString strNumber);

    bool dialOut(const QString &id, const QString &num);

private slots:
    void onGotRegisteredPhone (const GVRegisteredNumber &info);
    void onGotPhones();

    void onAllAccountsIdentified();

    void onDialoutCompleted();

private:
    bool ensurePhoneAccountFactory();
    void clearAllAccounts();

    QString chooseDefaultNumber();

public:
    GVNumModel *m_numModel;
    GVNumModel *m_ciModel;
    bool        m_ignoreSelectedNumberChanges;

    //! Number of times this was refreshed successfully
    quint32     s_Refresh;

private:
    IPhoneAccountFactory *m_acctFactory;

    friend class IMainWindow;
};

#endif // LIBGVPHONES_H
