#include "global.h"
#include "GVContactsTable.h"
#include "CaptchaWidget.h"

#include "Singletons.h"
#include "ContactsXmlHandler.h"

#include <QtDeclarative>

GVContactsTable::GVContactsTable (QWidget *parent)
: QDeclarativeView (parent)
, modelContacts (NULL)
, nwMgr (this)
, mutex(QMutex::Recursive)
, bLoggedIn(false)
, bRefreshRequested (false)
{
    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setDefaultWindowAttributes (this);
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

    if (NULL == modelContacts) {
        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        modelContacts = dbMain.newContactsModel ();
    }

    if (this->source ().isEmpty ()) {
        QDeclarativeContext *ctx = this->rootContext();
        ctx->setContextProperty ("contactsModel", modelContacts);

        this->setSource (QUrl ("qrc:/ContactsList.qml"));
        this->setResizeMode (QDeclarativeView::SizeRootObjectToView);

        QObject::connect (this, SIGNAL (sigCall   (const QString &)),
                          this, SLOT   (placeCall (const QString &)));
        QObject::connect (this, SIGNAL (sigText   (const QString &)),
                          this, SLOT   (sendSMS   (const QString &)));
    }
}//GVContactsTable::initModel

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
    if ((dbMain.getLastContactUpdate (dtUpdate)) && (dtUpdate.isValid ()))
    {
        QString strUpdate = dtUpdate.toString ("yyyy-MM-ddThh:mm:ss");
        strUrl += QString ("&updated-min=%1&showdeleted=true").arg (strUpdate);
        bRefreshIsUpdate = true;
    }
    else
    {
        //TODO: Tell model to clear, not the DB
        dbMain.clearContacts ();
    }

    emit status ("Retrieving contacts", 0);
    getRequest (strUrl, this , SLOT (onGotContacts (QNetworkReply *)));
}//GVContactsTable::refreshContactsFromContactsAPI

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
GVContactsTable::placeCall (const QString &strNumber)
{
    emit callNumber (strNumber, QString ());
}//GVContactsTable::placeCall

void
GVContactsTable::sendSMS (const QString &strNumber)
{
    emit textANumber (strNumber, QString());
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
            qDebug ("Loading captcha");
            CaptchaWidget *captcha = new CaptchaWidget(strCaptchaUrl, this);
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

    bool rv = false;
    QString msg;
    QDateTime currDT = QDateTime::currentDateTime().toUTC ();
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setLastContactUpdate (currDT);

    QXmlInputSource inputSource;
    QXmlSimpleReader simpleReader;
    ContactsXmlHandler contactsHandler;

    do // Begin cleanup block (not a loop)
    {
        QByteArray byData = reply->readAll ();
        if (byData.contains ("Authorization required"))
        {
            msg = "Authorization failed.";
            break;
        }
        inputSource.setData (byData);

        QObject::connect (&contactsHandler, SIGNAL (status(const QString &, int)),
            this,            SIGNAL (status(const QString &, int)));

        QObject::connect (
            &contactsHandler, SIGNAL (oneContact (const ContactInfo &)),
             this,            SLOT   (gotOneContact (const ContactInfo &)));

        simpleReader.setContentHandler (&contactsHandler);
        simpleReader.setErrorHandler (&contactsHandler);

        rv = simpleReader.parse (&inputSource, false);

        msg = QString("Contact parsing done. total = %1. usable = %2")
                .arg (contactsHandler.getTotalContacts ())
                .arg (contactsHandler.getUsableContacts ());
    } while (0); // End cleanup block (not a loop)

//    QSqlTableModel *modelContacts = (QSqlTableModel *) ui->treeView->model ();
//    if (rv) {
//        emit status ("Contacts retrieved. Saving. This will take some time...");
//        modelContacts->submitAll ();
//        emit status ("Contacts committed to local database");
//    } else {
//        modelContacts->revertAll ();
//    }

//    modelContacts->select ();

    emit status (msg);
    emit allContacts (rv);

    reply->deleteLater ();
}//GVContactsTable::onGotContacts

void
GVContactsTable::gotOneContact (const ContactInfo &contactInfo)
{
    QMutexLocker locker(&mutex);
    if (contactInfo.bDeleted)
    {
        modelContacts->deleteContact (contactInfo);
    }
    else    // add or modify
    {
        modelContacts->insertContact (contactInfo);
    }

}//GVContactsTable::gotOneContact
