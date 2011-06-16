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
                                            QNetworkAccessManager &mgr,
                                            QObject *parent)
: QObject(parent)
, byData (data)
, bEmitLog (true)
, nwMgr (mgr)
, refCount (0)
{
}//ContactsParserObject::ContactsParserObject

void
ContactsParserObject::doWork ()
{
    QXmlInputSource inputSource;
    QXmlSimpleReader simpleReader;
    ContactsXmlHandler contactsHandler;
    contactsHandler.setEmitLog (bEmitLog);

    inputSource.setData (byData);

    QObject::connect (&contactsHandler, SIGNAL (status(const QString &, int)),
                       this,            SIGNAL (status(const QString &, int)));

    QObject::connect (
        &contactsHandler, SIGNAL (oneContact (const ContactInfo &)),
         this,            SIGNAL (onGotOneContact (const ContactInfo &)));

    simpleReader.setContentHandler (&contactsHandler);
    simpleReader.setErrorHandler (&contactsHandler);

    refCount.ref();
    bool rv = simpleReader.parse (&inputSource, false);

    if (!rv) {
        qDebug() << "Contacts parser failed to parse. Data =" << byData;
    }

    QString msg = QString("Total contacts: %1. Usable: %2")
            .arg (contactsHandler.getTotalContacts ())
            .arg (contactsHandler.getUsableContacts ());
    if (bEmitLog || (contactsHandler.getUsableContacts () != 0)) {
        emit status(msg);
    }

    if (!refCount.deref ()) {
        emit done(rv);
    }
}//ContactsParserObject::doWork

void
ContactsParserObject::setEmitLog (bool enable /*= true*/)
{
    bEmitLog = enable;
}//ContactsParserObject::setEmitLog

void
ContactsParserObject::onGotOneContact (const ContactInfo &contactInfo)
{
    if (contactInfo.bDeleted) {
        emit gotOneContact (contactInfo);
        if (!refCount.deref ()) {
            emit done(rv);
        }
        return;
    }

    QNetworkRequest request = createRequest (contactInfo.hrefPhoto);
    QNetworkReply *reply = nwMgr.get (request);
    PhotoReplyTracker *tracker =
    new PhotoReplyTracker(contactInfo, reply, this);
    connect (reply, SIGNAL(finished()), tracker, SLOT(onFinished()));
    connect (tracker, SIGNAL(gotOneContact(const ContactInfo &)),
             this   , SIGNAL(gotOneContact(const ContactInfo &)));
}

PhotoReplyTracker::PhotoReplyTracker(const ContactInfo &ci,
                                           QNetworkReply *r,
                                           QObject *parent /*= NULL*/)
: QObject(parent)
, reply(r)
, contactInfo(ci)
{
}//PhotoReplyTracker::PhotoReplyTracker

void
PhotoReplyTracker::onFinished()
{
    QByteArray ba = reply->readAll ();
    do { // Begin cleanup block (not a loop)
        if (QNetworkReply::NoError != reply->error ()) {
            qDebug() << "Error in photo nw response:" << (int)reply->error();
            break;
        }

        if (0 == ba.length ()) {
            qDebug("Zero length response");
            break;
        }

        QString strTemplate = QDir::tempPath ()
                            + QDir::separator ()
                            + "qgv_XXXXXX.tmp.jpg";
        QTemporaryFile tempFile (strTemplate);
        if (!tempFile.open ()) {
            qWarning ("Failed to get a temp file name");
            break;
        }

        qDebug() << "Temp photo file =" << tempFile.fileName ();

        tempFile.setAutoRemove (false);
        tempFile.write (ba);

        contactInfo.strPhotoPath = tempFile.fileName ();

        emit gotOneContact (contactInfo);
    } while (0); // End cleanup block (not a loop)

    reply->deleteLater ();
    this->deleteLater ();
}//PhotoReplyTracker::onFinished
