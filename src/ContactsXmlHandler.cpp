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
    QString msg;

    do // Begin cleanup block (not a loop)
    {
        strCurrentChars.clear ();

        if (localName == "entry")
        {
            bEntryStarted = true;
            countContacts++;

            currInfo.init ();
            break;
        }

        if (!bEntryStarted)
        {
            break;
        }

        if (localName == "phoneNumber")
        {
            currPhone.init ();
            QString rel = atts.value ("rel");
            if (rel.endsWith ("mobile"))
            {
                currPhone.Type = PType_Mobile;
            }
            else if (rel.endsWith ("home"))
            {
                currPhone.Type = PType_Home;
            }
            else
            {
                currPhone.Type = PType_Other;
            }
        }

        if (qName == "gd:deleted")
        {
            currInfo.bDeleted = true;
        }
    } while (0); // End cleanup block (not a loop)
    return (true);
}//ContactsXmlHandler::startElement

bool
ContactsXmlHandler::endElement (const QString & /*namespaceURI*/,
                                const QString &localName        ,
                                const QString & /*qName*/       )
{
    QString msg;
    do // Begin cleanup block (not a loop)
    {
        if (localName == "id") {
            currInfo.strId = strCurrentChars;
            break;
        }
        if (localName == "title") {
            currInfo.strTitle = strCurrentChars;
            break;
        }
        if (localName == "phoneNumber") {
            currPhone.strNumber = strCurrentChars;
            currInfo.arrPhones += currPhone;
            break;
        }
        if (localName == "content") {
            currInfo.strNotes = strCurrentChars;
            break;
        }

        if (localName != "entry") break;
        // If execution reaches here then it means it's the end of an entry.
        bEntryStarted = false;

        if (currInfo.bDeleted)
        {
            if (bEmitLog)
                qDebug () << QString("GV reports deleted contact : "
                                     "id = %1, name = %2")
                                .arg (currInfo.strId)
                                .arg (currInfo.strTitle);
        }

        if ((0 == currInfo.arrPhones.size ()) && (!currInfo.bDeleted))
        {
            if (bEmitLog)
                qDebug() << "Contact does not have any phone numbers :"
                         << currInfo.strTitle;
            // Just in case, delete it!
            currInfo.bDeleted = true;
        } else {
            if (bEmitLog)
                qDebug() << "Pulled one contact from Google with valid numbers:"
                         << currInfo.strTitle;
        }

#if !NO_DBGINFO
        // For debug
        if (currInfo.strTitle.contains ("Jamie", Qt::CaseInsensitive)) {
            if (bEmitLog) qDebug ("It's Jamie");
        }
#endif

        countUsableContacts ++;
        emit oneContact (currInfo);
    } while (0); // End cleanup block (not a loop)
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
ContactsXmlHandler::setEmitLog (bool enable /*= true*/)
{
    bEmitLog = enable;
}//ContactsXmlHandler::setEmitLog
