/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016 Yuvraaj Kelkar

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

#ifndef __SKYPEOBSERVER_H__
#define __SKYPEOBSERVER_H__

#include "IObserver.h"
#include "SkypeClient.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class SkypeObserver : public IObserver
{
    Q_OBJECT

public:
    QString name();

protected:
    SkypeObserver (QObject *parent = NULL);
    virtual ~SkypeObserver(void);

    void startMonitoring (const QString &strC);
    void stopMonitoring ();

    void initClient ();

private slots:
    //! Invoked when Skype is initialized
    void onInitSkype (bool bSuccess, const QVariantList &params);

    //! Invoked when the status changes for a skype call
    void onCallStatusChanged (uint callId, const QString &strStatus);
    //! Invoked when the call info is fully retrieved
    void onCallInfoDone (bool bOk, const QVariantList &params);

private:
    //! The skype client if one can be created
    SkypeClient    *m_skypeClient;
    //! Array of active call IDs
    QVector<ulong>  arrCalls;

    friend class ObserverFactory;
};

#endif //__SKYPEOBSERVER_H__
