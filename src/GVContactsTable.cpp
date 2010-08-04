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
GVContactsTable::refreshContacts ()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QMutexLocker locker(&mutex);
    QString strUpdate, strUrl;

    strSavedLink.clear ();
    strUrl = QString ("http://www.google.com/m8/feeds/contacts/%1/full"
                      "?max-results=10000")
                        .arg (strUser);

    bRefreshIsUpdate = false;
    if ((dbMain.getLastContactUpdate (strUpdate)) && (0 != strUpdate.size ()))
    {
        strUrl += QString ("&updated-min=%1&showdeleted=true").arg (strUpdate);
        bRefreshIsUpdate = true;
        nContacts = this->model()->rowCount();
    }
    else
    {
        dbMain.clearContacts ();
        nContacts = 0;
    }

    emit status ("Retrieving contacts", 0);
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
    QObject::disconnect (&nwMgr, SIGNAL (finished (QNetworkReply *)),
                          this , SLOT   (onGotContacts (QNetworkReply *)));
    emit status ("Contacts retrieved, parsing", 0);

    QDateTime currDT = QDateTime::currentDateTime().toUTC ();
    QString strDateTime = QString ("%1T%2")
                            .arg (currDT.toString ("yyyy-MM-dd"))
                            .arg (currDT.toString ("hh:mm:ss"));
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setLastContactUpdate (strDateTime);

    QXmlInputSource inputSource (reply);
    QXmlSimpleReader simpleReader;
    ContactsXmlHandler contactsHandler;

    QObject::connect (&contactsHandler, SIGNAL (log(const QString &, int)),
                       this,            SIGNAL (log(const QString &, int)));
    QObject::connect (&contactsHandler, SIGNAL (status(const QString &, int)),
                       this,            SIGNAL (status(const QString &, int)));

    QObject::connect (
        &contactsHandler, SIGNAL (oneContact (const ContactInfo &)),
         this,            SLOT   (gotOneContact (const ContactInfo &)));

    simpleReader.setContentHandler (&contactsHandler);
    simpleReader.setErrorHandler (&contactsHandler);

    bool rv = simpleReader.parse (&inputSource, false);

    QString msg = QString("Contact parsing done. total = %1. usable = %2")
                    .arg (contactsHandler.getTotalContacts ())
                    .arg (contactsHandler.getUsableContacts ());
    emit status (msg);
    emit allContacts (rv);

    reply->deleteLater ();
}//GVContactsTable::onGotContacts

void
GVContactsTable::gotOneContact (const ContactInfo &contactInfo)
{
    QMutexLocker locker(&mutex);

    QModelIndexList listMatches;
    char whatWork = 0;
    do // Begin cleanup block (not a loop)
    {
        if (!bRefreshIsUpdate)
        {
            whatWork = 'a'; // add
            break;
        }

        QModelIndex idxStart = this->model()->index (0,1);
        if (!idxStart.isValid ())
        {
            emit log ("Invalid starting index for contact update", 3);
            whatWork = 'a'; // add
            break;
        }

        listMatches =
        this->model()->match (idxStart, Qt::DisplayRole, contactInfo.strId);
        if (0 == listMatches.size ())
        {
            emit log ("No matches found for ID to update contact");
            whatWork = 'a'; // add
            break;
        }

        if (contactInfo.bDeleted)
        {
            whatWork = 'd'; // delete
            break;
        }

        whatWork = 'm';

    } while (0); // End cleanup block (not a loop)

    GVContactInfo gvContactInfo;
    convert (contactInfo, gvContactInfo);

    if ('a' == whatWork)
    {
        emit oneContact (nContacts, contactInfo);
        nContacts++;
        return;
    }

    QModelIndex idxId = listMatches[0];
    if (!idxId.isValid ())
    {
        emit log ("Invalid title ID during delete or modify contact");
        return;
    }

    if ('m' == whatWork)
    {
        // update the model entry
        QModelIndex idxName = idxId.sibling (idxId.row (), 0);
        this->model()->setData (idxName, contactInfo.strTitle);

        // update dbMain.CACHE

        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        dbMain.putContactInfo (gvContactInfo);
    }
    else if ('d' == whatWork)
    {
        this->model()->removeRow (idxId.row ());
        nContacts--;
    }

}//GVContactsTable::gotOneContact

bool
GVContactsTable::convert (const ContactInfo &cInfo, GVContactInfo &gvcInfo)
{
    gvcInfo.strLink = cInfo.strId;
    gvcInfo.strName = cInfo.strTitle;

    foreach (PhoneInfo pInfo, cInfo.arrPhones)
    {
        GVContactNumber gvcn;
        switch (pInfo.Type)
        {
        case PType_Mobile:
            gvcn.chType = 'M';
            break;
        case PType_Home:
            gvcn.chType = 'H';
            break;
        case PType_Other:
            gvcn.chType = 'O';
            break;
        default:
            gvcn.chType = '?';
            break;
        }

        gvcn.strNumber = pInfo.strNumber;

        gvcInfo.arrPhones += gvcn;
    }

    return (true);
}//GVContactsTable::convert
