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

#ifndef __SKYPECLIENT_H__
#define __SKYPECLIENT_H__

#include "global.h"
#include "SkypeClientFactory.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

enum Skype_Work {
    SW_Nothing = 0,
    SW_Connect,         // No params
    SW_InitiateCall,    // Target contacts (1 or more)
    SW_GetContacts,     // No params
    SW_GetCallInfo,     // Call id
    SW_SendDtmf         // Send DTMF to the current call
};

struct Skype_CallInfo
{
    Skype_CallInfo () {init ();}
    void init () {
        bPSTN_valid = bIncoming_valid = bPartnerHandle_valid =
        bPartnerName_valid = bSelfNumber_valid = bPSTN = bIncoming = bInprogress
        = false;

        strPartnerHandle.clear ();
        strPartnerName.clear ();
        strSelfNumber.clear ();
        strDeferredTones.clear ();
    }

    quint32 bPSTN_valid             :1;
    quint32 bIncoming_valid         :1;
    quint32 bPartnerHandle_valid    :1;
    quint32 bPartnerName_valid      :1;
    quint32 bSelfNumber_valid       :1;

    //! true = PSTN, false = P2P
    quint32 bPSTN                   :1;
    // true = incoming, false = outgoing
    quint32 bIncoming               :1;
    quint32 bInprogress             :1;

    //! The handle of the other party
    QString strPartnerHandle;
    //! The display name of the other party
    QString strPartnerName;
    //! If this is a PSTN call, what is my number?
    QString strSelfNumber;
    // Any DTMF tones to send after call becomes in progress?
    QString strDeferredTones;
};
typedef QMap<ulong, Skype_CallInfo> Skype_CallInfoMap;
Q_DECLARE_METATYPE (Skype_CallInfo)

struct Skype_WorkItem
{
    Skype_WorkItem()
    {
        init ();
    }

    void init ()
    {
        whatwork = SW_Nothing;
        arrParams.clear ();
        receiver = NULL;
        method   = NULL;
        bCancel  = false;
    }

    Skype_Work      whatwork;
    QVariantList    arrParams;

    bool            bCancel;

    // Callback
    QObject        *receiver;
    const char     *method;
};

class SkypeClient : public QThread
{
    Q_OBJECT

protected:
    SkypeClient(const QString &name, QObject *parent = 0);
    int addRef ();
    int decRef ();

public:
    bool enqueueWork (Skype_Work whatwork, const QVariantList &params,
                      QObject   *receiver, const char         *method);
    bool isConnected ();

signals:
    //! Status emitter
    void status (const QString &txt, int timeout = 0);
    //! Emitted at the end of every work item
    void workCompleted (bool bSuccess, const QVariantList &params);

    //! Emitted for any change in a call
    void callStatusChanged (uint callId, const QString &strStatus);
    //! Emitted for every single contact
    void gotSingleContact (const QString &strContact);

    //! Emitted at the end of every internal work (PRIVATE)
    void internalCompleted (int status, const QString &strOutput);

    void connectedChanged(bool bChange);

protected slots:
    //! Invoked when the call initiation is over
    void callInitiated (int status, const QString &strOutput);

    //! Invoked when the contacts are returned
    void onGotContacts (int status, const QString &strOutput);

    //! Invoked as part of callInfo processing: after getting call type
    void onCI_GetType (uint incomingCallId, const QString &strOutput,
                       bool bNext = false);

    //! Invoked as part of callInfo processing: after getting partner handle
    void onCI_GetPH (uint incomingCallId, const QString &strOutput,
                     bool bNext = false);
    //! Invoked as part of callInfo processing: after getting partner name
    void onCI_GetPName (uint incomingCallId, const QString &strOutput,
                        bool bNext = false);
    //! Invoked as part of callInfo processing: after getting partner name
    void onCI_GetTarget (uint incomingCallId, const QString &strOutput,
                         bool bNext = false);

protected:
    //! Call for default handling. Returns true if handled.
    bool skypeNotifyPre (const QString &strData);
    //! Connect to the skype service
    virtual bool ensureConnected ();
    //! Invoke a command in a platform dependent manner
    virtual bool invoke (const QString &strCommand) = 0;
    //! Do next work or sleep because some work is pending
    void doNextWork ();
    //! Complete work and begin next work
    void completeCurrentWork (Skype_Work whatwork, bool bOk);
    //! Debug function to get the name of a work type
    QString getNameForWork (Skype_Work whatwork);

    //! Internal function to call one or more contacts.
    bool initiateCall (const QString &strTarget);
    //! Internal function to collect all passed contacts and send to th real function
    bool initiateCall ();

    //! Ask skype for all its contacts
    void getContacts ();

    //! Get all call related information
    void getCallInfo ();

    //! Send DTMF to current call
    void sendDTMF();
    bool sendDTMF(QString strTones, bool bFirstTime);

protected:
    //! Name of the client
    QString strName;

    //! Have we initiated connection?
    bool    bConnected;

    //! Release this semaphore once for every pending work
    QSemaphore              semaphore;

    //! The mutex that locks all the members below
    QMutex                  mutex;
    //! The list of work
    QList<Skype_WorkItem>   workList;
    //! Whats the current work being done
    Skype_WorkItem          workCurrent;

    int                     nRefCount;

    //! This has the info of every call thats currently on.
    Skype_CallInfoMap       mapCallInfo;

    friend class SkypeClientFactory;
};

#endif // __SKYPECLIENT_H__
