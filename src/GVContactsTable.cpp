#include "global.h"
#include "GVContactsTable.h"
#include "Singletons.h"
#include "CaptchaWidget.h"
#include "ContactsXmlHandler.h"

GVContactsTable::GVContactsTable (QWidget *parent)
: QTreeView(parent)
, nwMgr (this)
, actRefresh("&Refresh", this)
, mnuContext("Action", this)
, actPlaceCall("Call", this)
, actSendSMS("SMS", this)
, mutex(QMutex::Recursive)
, bLoggedIn(false)
{
    // Not modifyable
    this->setEditTriggers (QAbstractItemView::NoEditTriggers);
    // Only one item selectable at a time.
    this->setSelectionMode (QAbstractItemView::SingleSelection);
    // Alternating colors = on
    this->setAlternatingRowColors (true);
    // Don't show the (un)collapse sign
    this->setRootIsDecorated (false);
    // Hide the header
    this->setHeaderHidden (true);

    // Prepare the menu actions
    mnuContext.addAction (&actPlaceCall);
    mnuContext.addAction (&actSendSMS);

    // actPlaceCall.triggered -> this.placeCall
    QObject::connect (&actPlaceCall, SIGNAL (triggered ()),
                       this        , SLOT   (placeCall ()));
    // actSendSMS.triggered -> this.sendSMS
    QObject::connect (&actSendSMS, SIGNAL (triggered ()),
                       this      , SLOT   (sendSMS ()));

    QKeySequence keyRefresh(Qt::ControlModifier + Qt::Key_R);
    actRefresh.setShortcut (keyRefresh);
    QObject::connect (&actRefresh, SIGNAL (triggered ()),
                       this      , SLOT   (refreshContacts ()));

    // this.activated -> this.activatedContact
    QObject::connect (
        this, SIGNAL (activated        (const QModelIndex &)),
        this, SLOT   (activatedContact (const QModelIndex &)));
}//GVContactsTable::GVContactsTable

void
GVContactsTable::refreshContacts ()
{
    refreshContactsFromContactsAPI ();
}//GVContactsTable::refreshContacts

#if 0
void
GVContactsTable::refreshContactsFromWebGV ()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QMutexLocker locker(&mutex);
    dbMain.clearContacts ();
    strSavedLink.clear ();

    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QVariantList l;
    nContacts = 0;
    QObject::connect (
        &webPage, SIGNAL (gotContact (const QString &, const QString &)),
         this   , SLOT   (gotContact (const QString &, const QString &)));
    emit status ("Retrieving all contacts...", 0);
    if (!webPage.enqueueWork (GVAW_getAllContacts, l, this,
            SLOT (getContactsDone (bool, const QVariantList &))))
    {
        getContactsDone (false, l);
    }
}//GVContactsTable::refreshContactsFromWebGV

void
GVContactsTable::gotContact (const QString &strName, const QString &strLink)
{
    emit oneContact (nContacts, strName, strLink);

    QMutexLocker locker(&mutex);
    nContacts++;
}//GVContactsTable::gotContact

void
GVContactsTable::getContactsDone (bool bOk, const QVariantList &)
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QObject::disconnect(
        &webPage, SIGNAL (gotContact(const QString &, const QString &)),
        this    , SLOT   (gotContact(const QString &, const QString &)));

    emit allContacts (bOk);
}//GVContactsTable::getContactsDone
#endif

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

    QObject::connect (&nwMgr   , SIGNAL (finished (QNetworkReply *)),
                       receiver, method);
    QNetworkReply *reply = nwMgr.get (request);
    return (reply);
}//GVContactsTable::getRequest

void
GVContactsTable::refreshContactsFromContactsAPI ()
{
    QString strUrl = QString ("http://www.google.com/m8/feeds/contacts/%1/full"
                              "?max-results=10000")
                        .arg (strUser);
    //strUrl.replace ('@', "%40");

    getRequest (strUrl, this , SLOT (onGotContacts (QNetworkReply *)));
}//GVContactsTable::refreshContactsFromContactsAPI

void
GVContactsTable::updateMenu (QMenuBar *menuBar)
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        return;
    }

    menuBar->addAction (&actRefresh);
    menuBar->addAction (&actPlaceCall);
    menuBar->addAction (&actSendSMS);
}//GVContactsTable::updateMenu

void
GVContactsTable::loginSuccess ()
{
    QMutexLocker locker(&mutex);
    bLoggedIn = true;

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
GVContactsTable::activatedContact (const QModelIndex &)
{
    placeCall ();
}//GVContactsTable::activatedContact

void
GVContactsTable::selectionChanged (const QItemSelection &selected,
                                   const QItemSelection &/*deselected*/)
{
    QModelIndexList listModels = selected.indexes ();
    if (0 == listModels.size ())
    {
        strSavedLink.clear ();
        return;
    }
    QModelIndex linkIndex = listModels[0].model()->index (listModels[0].row(),
                                                          1);
    strSavedLink = linkIndex.data(Qt::EditRole).toString();
    if (strSavedLink.isEmpty ())
    {
        emit log ("Failed to get contact information", 3);
    }
}//GVContactsTable::selectionChanged

void
GVContactsTable::contextMenuEvent (QContextMenuEvent * event)
{
    mnuContext.popup (event->globalPos ());
}//GVContactsTable::contextMenuEvent

void
GVContactsTable::placeCall ()
{
    QMutexLocker locker(&mutex);
    if (0 != strSavedLink.size ())
    {
        emit callNameLink (strSavedLink, "");
    }
    else
    {
        emit status ("Nothing selected");
    }
}//GVContactsTable::placeCall

void
GVContactsTable::sendSMS ()
{
    QMutexLocker locker(&mutex);
    if (0 != strSavedLink.size ())
    {
        emit sendSMSToNameLink (strSavedLink, "");
    }
    else
    {
        emit status ("Nothing selected");
    }
}//GVContactsTable::sendSMS

void
GVContactsTable::setUserPass (const QString &strU, const QString &strP)
{
    QMutexLocker locker(&mutex);
    strUser = strU;
    strPass = strP;
}//GVContactsTable::setUserPass

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
            else
            {
                if (2 == arrPair.size ())
                {
                    msg = QString ("key = \"%1\", value = \"%2\"")
                            .arg (arrPair[0]).arg (arrPair[1]);
                }
                else if (1 == arrPair.size ())
                {
                    msg = QString ("key = \"%1\"")
                            .arg (arrPair[0]);
                }
                else
                {
                    msg = "Empty pair?!?";
                }
                emit log (msg);
            }
        }

        if (0 != strCaptchaUrl.size ())
        {
            strCaptchaUrl = "http://www.google.com/accounts/"
                          + strCaptchaUrl;
            emit log ("Loading captcha");
            CaptchaWidget *captcha = new CaptchaWidget(strCaptchaUrl, this);
            QObject::connect (
                captcha, SIGNAL (done (bool, const QString &)),
                this   , SLOT   (onCaptchaDone (bool, const QString &)));
            break;
        }

        if (0 == strGoogleAuth.size ())
        {
            emit log ("Failed to login!!");
            break;
        }

        emit log ("Login success");
    } while (0); // End cleanup block (not a loop)
    reply->deleteLater ();
}//GVContactsTable::onLoginResponse

void
GVContactsTable::onCaptchaDone (bool bOk, const QString &strCaptcha)
{
    // No point disconnecting anything because the widget is going to delete
    // itself anyway.

    do { // Begin cleanup block (not a loop)
        if (!bOk)
        {
            log ("Captcha failed");
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
    QXmlInputSource inputSource (reply);
    QXmlSimpleReader simpleReader;
    ContactsXmlHandler contactsHandler;

    QObject::connect (&contactsHandler, SIGNAL (log(const QString &, int)),
                       this,            SLOT   (log(const QString &, int)));
    QObject::connect (&contactsHandler, SIGNAL (status(const QString &, int)),
                       this,            SLOT   (status(const QString &, int)));

    QObject::connect (
        &contactsHandler, SIGNAL (oneContact (const ContactInfo &)),
         this,            SLOT   (gotOneContact (const ContactInfo &)));

    simpleReader.setContentHandler (&contactsHandler);
    simpleReader.setErrorHandler (&contactsHandler);

    simpleReader.parse (&inputSource, false);

    QString msg = QString("All done. total = %1. usable = %2")
                    .arg (contactsHandler.getTotalContacts ())
                    .arg (contactsHandler.getUsableContacts ());
    emit log (msg);

    reply->deleteLater ();
}//GVContactsTable::onGotContacts

void
GVContactsTable::gotOneContact (const ContactInfo &contactInfo)
{

}//GVContactsTable::gotOneContact
