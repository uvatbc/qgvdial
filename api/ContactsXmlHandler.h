/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2015  Yuvraaj Kelkar

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

#ifndef __CONTACTSXMLHANDLER_H__
#define __CONTACTSXMLHANDLER_H__

#include "api_common.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class ContactsXmlHandler : public QObject, public QXmlDefaultHandler
{
    Q_OBJECT

public:
    ContactsXmlHandler (QObject *parent = 0);
    virtual ~ContactsXmlHandler (void);

    quint32 getTotalContacts ();
    quint32 getUsableContacts ();

    void setEmitLog (bool enable = true);

signals:
    void status (const QString &text, int timeout = 2000);

    //! Emitted for every contact parsed from the XML
    void oneContact (ContactInfo contactInfo);

protected:
    bool startElement (const QString        &namespaceURI,
                       const QString        &localName   ,
                       const QString        &qName       ,
                       const QXmlAttributes &atts        );

    bool endElement (const QString &namespaceURI,
                     const QString &localName   ,
                     const QString &qName       );

    bool characters (const QString &ch);

protected:
    bool        bEntryStarted;

    quint32     countContacts;
    quint32     countUsableContacts;

    QString     strCurrentChars;

    PostalInfo  currPostal;
    EmailInfo   currEmail;
    PhoneInfo   currPhone;
    ContactInfo currInfo;

    bool        bEmitLog;
};

#endif //__CONTACTSXMLHANDLER_H__
