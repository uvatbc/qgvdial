#include "NotifyGVContactsTable.h"
#include "ContactsParserObject.h"

GVContactsTable::GVContactsTable (QObject *parent)
: QObject (parent)
, nwMgr (this)
, mutex(QMutex::Recursive)
, bLoggedIn(false)
, bRefreshRequested (false)
{
}//GVContactsTable::GVContactsTable

GVContactsTable::~GVContactsTable ()
{
}//GVContactsTable::~GVContactsTable

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
    bChangedSinceRefresh = false;

    QString strUrl = QString ("http://www.google.com/m8/feeds/contacts/%1/full"
                              "?max-results=10000")
                        .arg (strUser);

    bRefreshIsUpdate = false;
    if (dtUpdate.isValid ()) {
        QString strUpdate = dtUpdate.toString ("yyyy-MM-dd")
                          + "T"
                          + dtUpdate.toString ("hh:mm:ss");
        strUrl += QString ("&updated-min=%1&showdeleted=true").arg (strUpdate);
        bRefreshIsUpdate = true;
    }

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
            qCritical ("Google requested CAPTCHA!!");
            qApp->quit ();

            break;
        }

        if (0 == strGoogleAuth.size ())
        {
            qWarning ("Failed to login!!");
            break;
        }

        QMutexLocker locker (&mutex);
        bLoggedIn = true;

        if (bRefreshRequested)
        {
            refreshContacts ();
        }
    } while (0); // End cleanup block (not a loop)
    reply->deleteLater ();
}//GVContactsTable::onLoginResponse

void
GVContactsTable::onGotContacts (QNetworkReply *reply)
{
    QObject::disconnect (&nwMgr, SIGNAL (finished (QNetworkReply *)),
                          this , SLOT   (onGotContacts (QNetworkReply *)));

    do // Begin cleanup block (not a loop)
    {
        QByteArray byData = reply->readAll ();
        if (byData.contains ("Authorization required"))
        {
            emit status("Authorization failed.");
            break;
        }

#if 0
        QFile temp("dump.txt");
        temp.open (QIODevice::ReadWrite);
        temp.write (byData);
        temp.close ();
#endif

        dtUpdate = QDateTime::currentDateTime().toUTC ();

        QThread *workerThread = new QThread(this);
        ContactsParserObject *pObj = new ContactsParserObject(byData, strGoogleAuth);
        pObj->setEmitLog (false);
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
GVContactsTable::gotOneContact (const ContactInfo & /*contactInfo*/)
{
    QMutexLocker locker(&mutex);
    bChangedSinceRefresh = true;
}//GVContactsTable::gotOneContact

void
GVContactsTable::onContactsParsed (bool rv)
{
    emit allContacts (bChangedSinceRefresh, rv);
}//GVContactsTable::onContactsParsed
