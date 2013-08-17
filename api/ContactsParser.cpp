/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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

#include "ContactsParser.h"
#include "ContactsXmlHandler.h"

ContactsParser::ContactsParser (QByteArray data,
                                            QObject *parent /*= 0*/)
: QObject(parent)
, byData (data)
, bEmitLog (true)
{
}//ContactsParser::ContactsParser

void
ContactsParser::doWork ()
{
    bool rv;
    QXmlInputSource inputSource;
    QXmlSimpleReader simpleReader;
    ContactsXmlHandler contactsHandler;
    contactsHandler.setEmitLog (bEmitLog);

    inputSource.setData (byData);

    rv = connect (&contactsHandler, SIGNAL(status(const QString&,int)),
                  this,             SIGNAL(status(const QString&,int)));
    Q_ASSERT(rv);
    rv = connect (
            &contactsHandler, SIGNAL   (oneContact(const ContactInfo&)),
            this,            SIGNAL(gotOneContact(const ContactInfo&)));
    Q_ASSERT(rv);

    simpleReader.setContentHandler (&contactsHandler);
    simpleReader.setErrorHandler (&contactsHandler);

    rv = simpleReader.parse (&inputSource, false);

    if (!rv) {
        Q_WARN(QString("Contacts parser failed to parse. Data = %1")
               .arg (QString(byData)));
    }

    quint32 total = contactsHandler.getTotalContacts ();
    quint32 usable = contactsHandler.getUsableContacts ();
    emit done(rv, total, usable);

    if (bEmitLog || (contactsHandler.getUsableContacts () != 0)) {
        QString msg = QString("Total contacts: %1. Usable: %2")
                .arg (total).arg (usable);
        emit status(msg);
    }
}//ContactsParser::doWork

ContactsParser::~ContactsParser()
{
}//ContactsParser::~ContactsParser

void
ContactsParser::setEmitLog (bool enable /*= true*/)
{
    bEmitLog = enable;
}//ContactsParser::setEmitLog
