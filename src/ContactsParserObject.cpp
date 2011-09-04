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
                                            QObject *parent /*= 0*/)
: QObject(parent)
, byData (data)
, bEmitLog (true)
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
         this,            SIGNAL(gotOneContact (const ContactInfo &)));
    Q_ASSERT(rv);

    simpleReader.setContentHandler (&contactsHandler);
    simpleReader.setErrorHandler (&contactsHandler);

    rv = simpleReader.parse (&inputSource, false);

    if (!rv) {
        qWarning() << "Contacts parser failed to parse. Data =" << byData;
    }

    quint32 total = contactsHandler.getTotalContacts ();
    quint32 usable = contactsHandler.getUsableContacts ();
    emit done(rv, total, usable);

    if (bEmitLog || (contactsHandler.getUsableContacts () != 0)) {
        QString msg = QString("Total contacts: %1. Usable: %2")
                .arg (total).arg (usable);
        emit status(msg);
    }
}//ContactsParserObject::doWork

ContactsParserObject::~ContactsParserObject()
{
}//ContactsParserObject::~ContactsParserObject

void
ContactsParserObject::setEmitLog (bool enable /*= true*/)
{
    bEmitLog = enable;
}//ContactsParserObject::setEmitLog

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
    do { // Begin cleanup block (not a loop)
        if (aborted) {
            qWarning ("Reply aborted!");
            break;
        }

        QByteArray ba = reply->readAll ();

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
    onFinished ();
}//PhotoReplyTracker::onResponseTimeout
