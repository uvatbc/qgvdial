/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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

#include "ContactsXmlHandler.h"

ContactsXmlHandler::ContactsXmlHandler (QObject *parent)
: QObject(parent)
, QXmlDefaultHandler ()
, bEntryStarted (false)
, countContacts (0)
, countUsableContacts (0)
, bEmitLog (true)
{
}//ContactsXmlHandler::ContactsXmlHandler

ContactsXmlHandler::~ContactsXmlHandler(void)
{
}//ContactsXmlHandler::~ContactsXmlHandler

bool
ContactsXmlHandler::startElement (const QString        & /*namespaceURI*/,
                                  const QString        &localName   ,
                                  const QString        &qName       ,
                                  const QXmlAttributes &atts        )
{
    QString rel;
    strCurrentChars.clear ();

    do {
        if (localName == "entry") {
            bEntryStarted = true;
            countContacts++;

            currInfo.init ();
            break;
        }

        if (!bEntryStarted) {
            break;
        }

        if (localName == "phoneNumber") {
            currPhone.init ();
            rel = atts.value ("rel");
            if (rel.endsWith ("mobile")) {
                currPhone.Type = PType_Mobile;
            } else if (rel.endsWith ("home")) {
                currPhone.Type = PType_Home;
            } else if (rel.endsWith ("work")) {
                currPhone.Type = PType_Work;
            } else if (rel.endsWith ("pager")) {
                currPhone.Type = PType_Pager;
            } else {
                currPhone.Type = PType_Other;
            }
        }

        if (localName == "email") {
            currEmail.init ();
            currEmail.address = atts.value ("address");
            currEmail.primary = (atts.value ("primary") == "true");
            rel = atts.value ("rel");
            if (rel.endsWith ("home")) {
                currEmail.type = EType_Home;
            } else if (rel.endsWith ("work")) {
                currEmail.type = EType_Work;
            } else {
                currEmail.type = EType_Other;
            }
        }

        if (localName == "postalAddress") {
            currPostal.init ();
            rel = atts.value ("rel");
            if (rel.endsWith ("home")) {
                currPostal.type = PAType_Home;
            } else if (rel.endsWith ("work")) {
                currPostal.type = PAType_Work;
            } else {
                currPostal.type = PAType_Other;
            }
        }

        if (qName == "gd:deleted") {
            currInfo.bDeleted = true;
        }

        if (qName == "link") {
            rel = atts.value ("rel");
            if (rel.endsWith ("#photo")) {
                currInfo.hrefPhoto = atts.value ("href");
            }
        }
    } while (0);
    return (true);
}//ContactsXmlHandler::startElement

bool
ContactsXmlHandler::endElement (const QString & /*namespaceURI*/,
                                const QString &localName        ,
                                const QString & /*qName*/       )
{
    do {
        if (localName == "id") {
            currInfo.strId = strCurrentChars.trimmed ();
            break;
        }
        if (localName == "title") {
            currInfo.strTitle = strCurrentChars.trimmed ();
            break;
        }
        if (bEntryStarted) {
            if (localName == "postalAddress") {
                currPostal.address = strCurrentChars.trimmed ();
                currPostal.address.replace ("\\n", "\n");
                currInfo.arrPostal += currPostal;
                break;
            }
            if (localName == "email") {
                currInfo.arrEmails += currEmail;
                break;
            }
            if (localName == "phoneNumber") {
                currPhone.strNumber = strCurrentChars.trimmed ();
                currInfo.arrPhones += currPhone;
                break;
            }
        }
        if (localName == "content") {
            currInfo.strNotes = strCurrentChars.trimmed ();
            break;
        }
        if (localName == "updated") {
            currInfo.dtUpdate = QDateTime::fromString(strCurrentChars,
                                                      Qt::ISODate);
        }

        if (localName != "entry") break;
        // If execution reaches here then it means it's the end of an entry.
        bEntryStarted = false;

#ifdef DBG_VERBOSE
        // For debug
        if (currInfo.strTitle.contains ("Jamie", Qt::CaseInsensitive)) {
            if (bEmitLog) qDebug ("It's Jamie");
        }
#endif

        countUsableContacts ++;

        currInfo.hrefPhoto.replace ("%40", "@");

        emit oneContact (currInfo);

        currInfo.init ();
    } while (0);
    return (true);
}//ContactsXmlHandler::endElement

bool
ContactsXmlHandler::characters (const QString &ch)
{
    strCurrentChars += ch;

    return (true);
}//ContactsXmlHandler::characters

quint32
ContactsXmlHandler::getTotalContacts ()
{
    return (countContacts);
}//ContactsXmlHandler::getTotalContacts

quint32
ContactsXmlHandler::getUsableContacts ()
{
    return (countUsableContacts);
}//ContactsXmlHandler::getUsableContacts

void
ContactsXmlHandler::setEmitLog (bool enable /* = true*/)
{
    bEmitLog = enable;
}//ContactsXmlHandler::setEmitLog
