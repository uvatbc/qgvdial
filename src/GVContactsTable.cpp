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

#include "global.h"
#include "GVContactsTable.h"
#include "CaptchaWidget.h"

#include "Singletons.h"
#include "ContactsXmlHandler.h"
#include "ContactsModel.h"
#include "ContactsParserObject.h"

GVContactsTable::GVContactsTable (QObject *parent)
: QObject (parent)
, modelContacts (NULL)
, nwMgr (this)
, mutex(QMutex::Recursive)
, bLoggedIn(false)
, bRefreshRequested (false)
{
}//GVContactsTable::GVContactsTable

GVContactsTable::~GVContactsTable ()
{
    deinitModel ();
}//GVContactsTable::~GVContactsTable

void
GVContactsTable::deinitModel ()
{
    if (NULL != modelContacts) {
        delete modelContacts;
        modelContacts = NULL;
    }
}//GVContactsTable::deinitModel

void
GVContactsTable::initModel ()
{
    deinitModel ();

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    modelContacts = dbMain.newContactsModel ();

    emit setContactsModel (modelContacts);

    while (modelContacts->canFetchMore ()) {
        modelContacts->fetchMore ();
    }
}//GVContactsTable::initModel

QNetworkRequest
GVContactsTable::createRequest(QString strUrl)
{
    QUrl url (strUrl);
    QNetworkRequest request(url);
    request.setHeader (QNetworkRequest::ContentTypeHeader,
                       "application/x-www-form-urlencoded");
    if (0 != strGoogleAuth.size ())
    {
        QByteArray byAuth = QString("GoogleLogin auth=%1")
                                    .arg(strGoogleAuth).toAscii ();
        request.setRawHeader ("Authorization", byAuth);
    }

    return request;
}//GVContactsTable::createRequest

QNetworkReply *
GVContactsTable::postRequest (QString         strUrl,
                              QStringPairList arrPairs,
                              QObject        *receiver,
                              const char     *method)
{
    QStringList arrParams;
    foreach (QStringPair pairParam, arrPairs)
    {
        arrParams += QString("%1=%2")
                        .arg(pairParam.first)
                        .arg(pairParam.second);
    }
    QString strParams = arrParams.join ("&");

    QNetworkRequest request = createRequest (strUrl);
    QByteArray byPostData = strParams.toAscii ();

    QObject::connect (&nwMgr   , SIGNAL (finished (QNetworkReply *)),
                       receiver, method);
    QNetworkReply *reply = nwMgr.post (request, byPostData);
    return (reply);
}//GVContactsTable::postRequest

QNetworkReply *
GVContactsTable::getRequest (QString         strUrl,
                             QObject        *receiver,
                             const char     *method)
{
    QNetworkRequest request = createRequest (strUrl);
    QObject::connect (&nwMgr   , SIGNAL (finished (QNetworkReply *)),
                       receiver, method);
    QNetworkReply *reply = nwMgr.get (request);
    return (reply);
}//GVContactsTable::getRequest

void
GVContactsTable::refreshAllContacts ()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.clearLastContactUpdate ();

    refreshContacts ();
}//GVContactsTable::refreshAllContacts

void
GVContactsTable::refreshContacts ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        bRefreshRequested = true;
        return;
    }
    bRefreshRequested = false;

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QString strUrl;

    strUrl = QString ("http://www.google.com/m8/feeds/contacts/%1/full"
                      "?max-results=10000")
                        .arg (strUser);

    bRefreshIsUpdate = false;
    QDateTime dtUpdate;
    if ((dbMain.getLastContactUpdate (dtUpdate)) && (dtUpdate.isValid ())) {
        QString strUpdate = dtUpdate.toString ("yyyy-MM-dd")
                          + "T"
                          + dtUpdate.toString ("hh:mm:ss");
        strUrl += QString ("&updated-min=%1&showdeleted=true").arg (strUpdate);
        bRefreshIsUpdate = true;
    } else {
        modelContacts->clearAll ();
    }

    emit status ("Retrieving contacts", 0);
    getRequest (strUrl, this , SLOT (onGotContacts (QNetworkReply *)));
}//GVContactsTable::refreshContacts

void
GVContactsTable::setUserPass (const QString &strU, const QString &strP)
{
    QMutexLocker locker(&mutex);
    strUser = strU;
    strPass = strP;
}//GVContactsTable::setUserPass

void
GVContactsTable::loginSuccess ()
{
    QMutexLocker locker(&mutex);

    QStringPairList arrPairs;
    arrPairs += QStringPair("accountType", "GOOGLE");
    arrPairs += QStringPair("Email"      , strUser);
    arrPairs += QStringPair("Passwd"     , strPass);
    arrPairs += QStringPair("service"    , "cp"); // name for contacts service
    arrPairs += QStringPair("source"     , "MyCompany-qgvdial-ver01");
    postRequest (GV_CLIENTLOGIN, arrPairs,
                 this , SLOT (onLoginResponse (QNetworkReply *)));
}//GVContactsTable::loginSuccess

void
GVContactsTable::loggedOut ()
{
    QMutexLocker locker(&mutex);
    bLoggedIn = false;

    strGoogleAuth.clear ();
}//GVContactsTable::loggedOut

void
GVContactsTable::onLoginResponse (QNetworkReply *reply)
{
    QObject::disconnect (&nwMgr, SIGNAL (finished (QNetworkReply *)),
                          this , SLOT   (onLoginResponse (QNetworkReply *)));

    QString msg;
    QString strReply = reply->readAll ();
    QString strCaptchaToken, strCaptchaUrl;

    strGoogleAuth.clear ();
    do // Begin cleanup block (not a loop)
    {
        QStringList arrParsed = strReply.split ('\n');
        foreach (QString strPair, arrParsed)
        {
            QStringList arrPair = strPair.split ('=');
            if (arrPair[0] == "Auth")
            {
                strGoogleAuth = arrPair[1];
            }
            else if (arrPair[0] == "CaptchaToken")
            {
                strCaptchaToken = arrPair[1];
            }
            else if (arrPair[0] == "CaptchaUrl")
            {
                strCaptchaUrl = arrPair[1];
            }
        }

        if (0 != strCaptchaUrl.size ())
        {
            strCaptchaUrl = "http://www.google.com/accounts/"
                          + strCaptchaUrl;
            qDebug ("Loading captcha");
            CaptchaWidget *captcha = new CaptchaWidget(strCaptchaUrl);
            QObject::connect (
                captcha, SIGNAL (done (bool, const QString &)),
                this   , SLOT   (onCaptchaDone (bool, const QString &)));
            break;
        }

        if (0 == strGoogleAuth.size ())
        {
            qWarning ("Failed to login!!");
            break;
        }

        QMutexLocker locker (&mutex);
        bLoggedIn = true;

        qDebug ("Login success");

        if (bRefreshRequested)
        {
            refreshContacts ();
        }
    } while (0); // End cleanup block (not a loop)
    reply->deleteLater ();
}//GVContactsTable::onLoginResponse

void
GVContactsTable::onCaptchaDone (bool bOk, const QString & /*strCaptcha*/)
{
    // No point disconnecting anything because the widget is going to delete
    // itself anyway.

    do { // Begin cleanup block (not a loop)
        if (!bOk)
        {
            qWarning ("Captcha failed");
            break;
        }

        QStringPairList arrPairs;
        arrPairs += QStringPair("accountType", "GOOGLE");
        arrPairs += QStringPair("Email"      , strUser);
        arrPairs += QStringPair("Passwd"     , strPass);
        arrPairs += QStringPair("service"    , "grandcentral");
        arrPairs += QStringPair("source"     , "MyCompany-testapp16-ver01");
        //TODO: add captcha params
        postRequest (GV_CLIENTLOGIN, arrPairs,
                     this , SLOT (onLoginResponse (QNetworkReply *)));
    } while (0); // End cleanup block (not a loop)
}//GVContactsTable::onCaptchaDone

void
GVContactsTable::onGotContacts (QNetworkReply *reply)
{
    QObject::disconnect (&nwMgr, SIGNAL (finished (QNetworkReply *)),
                          this , SLOT   (onGotContacts (QNetworkReply *)));
    emit status ("Contacts retrieved, parsing", 0);

    do // Begin cleanup block (not a loop)
    {
        QByteArray byData = reply->readAll ();
        if (byData.contains ("Authorization required"))
        {
            emit status("Authorization failed.");
            break;
        }

#if 0
        QFile temp("contacts.txt");
        temp.open (QIODevice::ReadWrite);
        temp.write (byData);
        temp.close ();
#endif

        QDateTime currDT = QDateTime::currentDateTime().toUTC ();
        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        dbMain.setLastContactUpdate (currDT);

        QThread *workerThread = new QThread(this);
        ContactsParserObject *pObj = new ContactsParserObject(byData, nwMgr);
        pObj->moveToThread (workerThread);
        QObject::connect (workerThread, SIGNAL(started()),
                          pObj        , SLOT  (doWork()));
        QObject::connect (pObj, SIGNAL(done(bool)),
                          this, SLOT  (onContactsParsed(bool)));
        QObject::connect (pObj        , SIGNAL(done(bool)),
                          workerThread, SLOT  (quit()));
        QObject::connect (workerThread, SIGNAL(terminated()),
                          pObj        , SLOT  (deleteLater()));
        QObject::connect (workerThread, SIGNAL(terminated()),
                          workerThread, SLOT  (deleteLater()));
        QObject::connect (pObj, SIGNAL (status(const QString &, int)),
                          this, SIGNAL (status(const QString &, int)));
        QObject::connect (pObj, SIGNAL (gotOneContact (const ContactInfo &)),
                          this, SLOT   (gotOneContact (const ContactInfo &)));
        workerThread->start ();
    } while (0); // End cleanup block (not a loop)

    reply->deleteLater ();
}//GVContactsTable::onGotContacts

void
GVContactsTable::gotOneContact (const ContactInfo &contactInfo)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setQuickAndDirty();

    QMutexLocker locker(&mutex);
    if (contactInfo.bDeleted) {
        qDebug() << "Delete contact " << contactInfo.strTitle;
        modelContacts->deleteContact (contactInfo);
    } else {   // add or modify
        qDebug() << "Insert contact " << contactInfo.strTitle;
        modelContacts->insertContact (contactInfo);
    }

}//GVContactsTable::gotOneContact

void
GVContactsTable::onContactsParsed (bool rv)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setQuickAndDirty(false);
    // Tell the contacts model to refresh all.
    modelContacts->refresh ();
    emit allContacts (rv);
}//GVContactsTable::onContactsParsed

void
GVContactsTable::onSearchQueryChanged (const QString &query)
{
    modelContacts->refresh (query);
}//GVContactsTable::onSearchQuerychanged
