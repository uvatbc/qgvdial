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

ContactsParserObject::ContactsParserObject (QByteArray data, QObject *parent)
: QObject(parent)
, byData (data)
, bEmitLog (true)
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
         this,            SIGNAL (gotOneContact (const ContactInfo &)));

    simpleReader.setContentHandler (&contactsHandler);
    simpleReader.setErrorHandler (&contactsHandler);

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

    emit done(rv);
}//ContactsParserObject::doWork

void
ContactsParserObject::setEmitLog (bool enable /*= true*/)
{
    bEmitLog = enable;
}//ContactsParserObject::setEmitLog
