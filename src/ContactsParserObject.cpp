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

#include "ContactsParserObject.h"
#include "ContactsXmlHandler.h"

ContactsParserObject::ContactsParserObject (QByteArray data,
                                            const QString &strAuth,
                                            const QString &strTemp,
                                            QObject *parent)
: QObject(parent)
, byData (data)
, strTempStore (strTemp)
, bEmitLog (true)
, nwMgr (NULL)
, strGoogleAuth (strAuth)
, refCount (0)
{
}//ContactsParserObject::ContactsParserObject

void
ContactsParserObject::doWork ()
{
    bool rv;
    QXmlInputSource inputSource;
    QXmlSimpleReader simpleReader;
    ContactsXmlHandler contactsHandler;
    contactsHandler.setEmitLog (bEmitLog);

    inputSource.setData (byData);

    rv = connect (&contactsHandler, SIGNAL (status(const QString &, int)),
                       this,            SIGNAL (status(const QString &, int)));
    Q_ASSERT(rv);
    rv = connect (
        &contactsHandler, SIGNAL   (oneContact (const ContactInfo &)),
         this,            SLOT(onGotOneContact (const ContactInfo &)));
    Q_ASSERT(rv);

    simpleReader.setContentHandler (&contactsHandler);
    simpleReader.setErrorHandler (&contactsHandler);

    refCount.ref();
    rv = simpleReader.parse (&inputSource, false);

    if (!rv) {
        qDebug() << "Contacts parser failed to parse. Data =" << byData;
    }

    QString msg = QString("Total contacts: %1. Usable: %2")
            .arg (contactsHandler.getTotalContacts ())
            .arg (contactsHandler.getUsableContacts ());
    if (bEmitLog || (contactsHandler.getUsableContacts () != 0)) {
        emit status(msg);
    }

    this->decRef (rv);
}//ContactsParserObject::doWork

void
ContactsParserObject::decRef (bool rv /*= true*/)
{
    if (!refCount.deref ()) {
        if (bEmitLog) qDebug("All contacts and photos downloaded.");
        emit done(rv);
    }
}//ContactsParserObject::decRef

ContactsParserObject::~ContactsParserObject()
{
    if (NULL != nwMgr) {
        delete nwMgr;
        nwMgr = NULL;
    }
}//ContactsParserObject::~ContactsParserObject

void
ContactsParserObject::setEmitLog (bool enable /*= true*/)
{
    bEmitLog = enable;
}//ContactsParserObject::setEmitLog


QNetworkRequest
ContactsParserObject::createRequest(QString strUrl)
{
    QUrl url (strUrl);
    QNetworkRequest request(url);
    request.setHeader (QNetworkRequest::ContentTypeHeader,
                       "application/x-www-form-urlencoded");
    QByteArray byAuth = QString("GoogleLogin auth=%1")
                                .arg(strGoogleAuth).toAscii ();
    request.setRawHeader ("Authorization", byAuth);

    return request;
}//ContactsParserObject::createRequest

void
ContactsParserObject::onGotOneContact (const ContactInfo &contactInfo)
{
    refCount.ref ();

    if (contactInfo.bDeleted) {
        onGotOnePhoto (contactInfo);
        return;
    }

    if (strTempStore.isEmpty ()) {
        onGotOnePhoto (contactInfo);
        return;
    }

    if (NULL == nwMgr) {
        nwMgr = new QNetworkAccessManager(this);
    }

    QNetworkRequest request = createRequest (contactInfo.hrefPhoto);
    QNetworkReply *reply = nwMgr->get (request);
    PhotoReplyTracker *tracker =
    new PhotoReplyTracker(contactInfo, reply, strTempStore, this);
    connect (reply, SIGNAL(finished()), tracker, SLOT(onFinished()));
    connect (tracker, SIGNAL(gotOneContact(const ContactInfo &)),
             this   , SLOT  (onGotOnePhoto(const ContactInfo &)));
}//ContactsParserObject::onGotOneContact

void
ContactsParserObject::onGotOnePhoto (const ContactInfo &contactInfo)
{
    emit gotOneContact (contactInfo);
    this->decRef ();
}//ContactsParserObject::onGotOnePhoto

PhotoReplyTracker::PhotoReplyTracker(const ContactInfo &ci,
                                           QNetworkReply *r,
                                     const QString &strTemp,
                                           QObject *parent /*= NULL*/)
: QObject(parent)
, reply(r)
, strTempStore (strTemp)
, contactInfo(ci)
, responseTimeout(this)
, aborted (false)
{
    responseTimeout.setSingleShot (true);
    connect(&responseTimeout,SIGNAL(timeout()), this,SLOT(onResponseTimeout()));
    responseTimeout.start (30 * 1000);
}//PhotoReplyTracker::PhotoReplyTracker

void
PhotoReplyTracker::onFinished()
{
    responseTimeout.stop ();
    if (aborted) {
        qWarning ("Reply aborted!");
        return;
    }

    QByteArray ba = reply->readAll ();
    do { // Begin cleanup block (not a loop)
        QNetworkReply::NetworkError err = reply->error ();
        if (QNetworkReply::ContentNotFoundError == err) {
            break;
        }
        if (QNetworkReply::NoError != err) {
            qWarning() << "Error in photo nw response:" << (int)err;
            break;
        }

        if (0 == ba.length ()) {
            qWarning ("Zero length response for photo");
            break;
        }

        quint8 sPng[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
        QByteArray baPng((char*)sPng, sizeof(sPng));
        quint8 sBmp[] = {'B', 'M'};
        QByteArray baBmp((char*)sBmp, sizeof(sBmp));

        QString extension = "jpg";
        if (ba.startsWith (baPng)) {
            extension = "png";
        } else if (ba.startsWith (baBmp)) {
            extension = "bmp";
        }

        QString strTemplate = strTempStore + QDir::separator()
                            + tr("qgv_XXXXXX.tmp.") + extension;

        QTemporaryFile tempFile (strTemplate);
        if (!tempFile.open ()) {
            qWarning ("Failed to get a temp file name for the photo");
            break;
        }

        tempFile.setAutoRemove (false);
        tempFile.write (ba);

        contactInfo.strPhotoPath = tempFile.fileName ();
    } while (0); // End cleanup block (not a loop)
    reply->deleteLater ();
    this->deleteLater ();

    emit gotOneContact (contactInfo);
}//PhotoReplyTracker::onFinished

void
PhotoReplyTracker::onResponseTimeout()
{
    aborted = true;

    emit gotOneContact (contactInfo);

    reply->abort ();
    reply->deleteLater ();
    this->deleteLater ();
}//PhotoReplyTracker::onResponseTimeout
