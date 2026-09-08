/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2025 Yuvraaj Kelkar

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
ContactsXmlHandler::parse (const QByteArray &data)
{
    QXmlStreamReader reader(data);
    bEntryStarted = false;
    countContacts = 0;
    countUsableContacts = 0;
    strCurrentChars.clear ();
    currInfo.init ();

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            QString localName = reader.name().toString();
            QString qName = reader.qualifiedName().toString();
            strCurrentChars.clear ();

            if (localName == "entry") {
                bEntryStarted = true;
                countContacts++;
                currInfo.init ();
                continue;
            }

            if (!bEntryStarted) {
                continue;
            }

            const QXmlStreamAttributes &atts = reader.attributes();

            if (localName == "phoneNumber") {
                currPhone.init ();
                QString rel = atts.value ("rel").toString();
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
                currEmail.address = atts.value ("address").toString();
                currEmail.primary = (atts.value ("primary") == "true");
                QString rel = atts.value ("rel").toString();
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
                QString rel = atts.value ("rel").toString();
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
                QString rel = atts.value ("rel").toString();
                if (rel.endsWith ("#photo")) {
                    currInfo.hrefPhoto = atts.value ("href").toString();
                }
            }
        } else if (token == QXmlStreamReader::Characters) {
            strCurrentChars += reader.text().toString();
        } else if (token == QXmlStreamReader::EndElement) {
            QString localName = reader.name().toString();
            if (localName == "id") {
                currInfo.strId = strCurrentChars.trimmed ();
            }
            if (localName == "title") {
                currInfo.strTitle = strCurrentChars.trimmed ();
            }
            if (bEntryStarted) {
                if (localName == "postalAddress") {
                    currPostal.address = strCurrentChars.trimmed ();
                    currPostal.address.replace ("\\n", "\n");
                    currInfo.arrPostal += currPostal;
                }
                if (localName == "email") {
                    currInfo.arrEmails += currEmail;
                }
                if (localName == "phoneNumber") {
                    currPhone.strNumber = strCurrentChars.trimmed ();
                    currInfo.arrPhones += currPhone;
                }
            }
            if (localName == "content") {
                currInfo.strNotes = strCurrentChars.trimmed ();
            }
            if (localName == "updated") {
                currInfo.dtUpdate = QDateTime::fromString(strCurrentChars,
                                                          Qt::ISODate);
            }

            if (localName == "entry") {
                bEntryStarted = false;
                countUsableContacts ++;
                currInfo.hrefPhoto.replace ("%40", "@");
                emit oneContact (currInfo);
                currInfo.init ();
            }
        }
    }

    if (reader.hasError()) {
        if (bEmitLog) {
            Q_WARN(QString("Contacts XML parse error: %1").arg(reader.errorString()));
        }
        return false;
    }

    return true;
}//ContactsXmlHandler::parse

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
