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

#ifndef __GVWEBPAGE_H__
#define __GVWEBPAGE_H__

#include "global.h"
#include <QtWebKit>
#include <QtXmlPatterns>
#include "GVAccess.h"
#include "MobileWebPage.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class GVWebPage : public GVAccess
{
    Q_OBJECT

private:
    GVWebPage(QObject *parent = NULL);
    ~GVWebPage(void);

public:
    void setView (QWidget *view);
    QNetworkAccessManager *nwAccessMgr();

public slots:
    //! Invoked when the user presses cancel
    void userCancel ();

private slots:
    //! Invoked when the about:blank page has finished loading
    void aboutBlankDone (bool bOk);

    //! Invoked when the google voice login page is loaded
    void loginStage1 (bool bOk);
    //! Invoked when the google voice main page (after login) is loaded
    void loginStage2 (bool bOk);
    //! Invoked when we look at the inbox page for the rnr_se
    void loginStage3 (bool bOk);

    //! Invoked when the logout page has loaded
    void logoutDone (bool bOk);

    void onDataCallDone (QNetworkReply * reply);
    void onDataCallCanceled (QNetworkReply * reply);

    //! Invoked when the registered phone list XML is retrieved.
    void onGotPhonesListXML (QNetworkReply * reply);

    //! Invoked when GV responds with a inbox page.
    void onGotInboxXML (QNetworkReply *reply);

    //! Invoked when the garbage timer times out.
    void garbageTimerTimeout ();

    //! Invoked when the SMS main page is loaded
    void sendSMSResponse (QNetworkReply *reply);

    //! Invoked when th vmail is downloaded
    void onVmailDownloaded (QNetworkReply *reply);

    //! Invoked when the page timeout timer times out.
    void onPageTimeout ();
    //! Invoked when the page makes any progress.
    void onPageProgress (int progress);
    //! Invoked when the socket makes any transfers
    void onSocketXfer (qint64 bytesXfer, qint64 bytesTotal);

    //! Invoked when the inbox entry has been marked
    void onInboxEntryMarked(QNetworkReply *reply);

private:
    bool isOnline ();
    void getHostAndQuery (QString &strHost, QString &strQuery);
    void loadUrlString (const QString &strUrl);
    bool isLoadFailed (bool bOk);

    QNetworkReply *
    postRequest (QString            strUrl  ,
                 QStringPairList    arrPairs,
                 QString            strUA   ,
                 QObject           *receiver,
                 const char        *method  );

    //! Load the about:blank page
    bool aboutBlank ();
    //! Login to Google voice
    bool login ();
    //! Log out of Google voice
    bool logout ();
    //! Make a phone call to an arbitrary number
    bool dialCallback (bool bCallback);
    //! Get registered phones from the settings page
    bool getRegisteredPhones ();
    //! Begin the process to get inbox
    bool getInbox ();
    //! Create and send an inbox request
    bool sendInboxRequest ();
    //! This sends SMSes
    bool sendSMS ();
    //! Play a voicemail
    bool playVmail ();
    //! Mark an inbox entry as read
    bool markAsRead ();

    void cancelDataDial2 ();

    //! This aborts the timeout timer and calls the base completeWork
    virtual void completeCurrentWork (GVAccess_Work whatwork, bool bOk);
    //! Starts the timeout timer for NW requests
    void startTimerForReply(QNetworkReply *reply);

    bool parseInboxJson(const QDateTime &dtUpdate, const QString &strJson,
                        const QString &strHtml, bool &bGotOld, int &nNew,
                        qint32 &nUsableMsgs);
    bool parseMessageRow(QString &strRow, QString &strSmsRow);
    bool execXQuery(const QString &strQuery, QString &result);

private:
    //! The webkit page that does all our work
    MobileWebPage           webPage;

    //! The timer object that optimizes our memory usage
    QTimer                  garbageTimer;

    //! The starting inbox page
    int                     nFirstPage;

    //! The current page (contacts or inbox)
    int                     nCurrent;

#if MOBILITY_PRESENT
    //! We use this to check if we're online
    QNetworkConfigurationManager nwCfg;
#endif

    //! Timeout timer for web page loading
    QTimer                  pageTimeoutTimer;
    QNetworkReply          *pCurrentReply;

    //! This flag is to be set to distinguish between call out and call back.
    bool                    bIsCallback;

    //! This flag mentions if we are in cancellation
    bool                    bInDialCancel;

    friend class Singletons;
};

class MyXmlErrorHandler : public QAbstractMessageHandler
{
public:
    MyXmlErrorHandler(QObject *parent = NULL);

protected:
    void handleMessage (QtMsgType type, const QString &description,
                        const QUrl &identifier,
                        const QSourceLocation &sourceLocation);
};

#endif //__GVWEBPAGE_H__
