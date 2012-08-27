/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

#include "InboxModel.h"
#include "Singletons.h"
#include "GVApi.h"

InboxModel::InboxModel (QObject * parent)
: QSqlQueryModel (parent)
, strSelectType ("all")
, eSelectType (GVIE_Unknown)
{
    QHash<int, QByteArray> roles;
    roles[IN_TypeRole]  = "type";
    roles[IN_TimeRole]  = "time";
    roles[IN_NameRole]  = "name";
    roles[IN_NumberRole]= "number";
    roles[IN_Link]      = "link";
    roles[IN_TimeDetail]= "time_detail";
    roles[IN_SmsText]   = "smstext";
    roles[IN_ReadFlag]  = "is_read";
    setRoleNames(roles);
}//InboxModel::InboxModel

int
InboxModel::rowCount (const QModelIndex & /*parent = QModelIndex()*/) const
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    return (dbMain.getInboxCount (eSelectType));
}//InboxModel::rowCount

QVariant
InboxModel::data (const QModelIndex &index, int role) const
{
    QVariant var;

    do // Begin cleanup block (not a loop)
    {
        int column = -1;
        switch (role)
        {
        case Qt::DisplayRole:
        case Qt::EditRole:
            column = index.column ();
            break;
        case IN_Link:
            column = 0;
            break;
        case IN_TypeRole:
            column = 1;
            break;
        case IN_TimeRole:
        case IN_TimeDetail:
            column = 2;
            break;
        case IN_NameRole:
            column = 3;
            break;
        case IN_NumberRole:
            column = 4;
            break;
        case IN_ReadFlag:
            column = 5;
            break;
        case IN_SmsText:
            column = 6;
            break;
        }

        // Pick up the data from the base class
        var = QSqlQueryModel::data (index.sibling(index.row (), column),
                                    Qt::EditRole);

        if (0 == column)        // GV_IN_ID
        {
            QString strLink = var.toString ();
            if (0 == strLink.size ()) {
                qWarning ("Inbox: Invalid link: Blank!");
                var.clear ();
                break;
            }
        }
        else if (1 == column)   // GV_IN_TYPE
        {
            char chType = var.toChar().toAscii ();
            QString strDisp = type_to_string ((GVI_Entry_Type) chType);
            var.clear ();
            if (0 == strDisp.size ()) {
                qWarning () << "Inbox: Entry type could not be deciphered: "
                            << int(chType);
                break;
            }

            var = strDisp;
        }
        else if (2 == column)   // GV_IN_ATTIME
        {
            bool bOk = false;
            quint64 num = var.toULongLong (&bOk);
            var.clear ();
            if (!bOk) break;

            QDateTime dt = QDateTime::fromTime_t (num);
            QString strDisp;
            QDate currentDate = QDate::currentDate ();
            int daysTo = dt.daysTo (QDateTime::currentDateTime ());
            if (IN_TimeDetail == role) {
                if (0 == daysTo) {
                    strDisp = dt.toString ("hh:mm:ss")  + " today";
                } else if (1 == daysTo) {
                    strDisp = dt.toString ("hh:mm:ss") + " yesterday";
                } else {
                    strDisp = dt.toString ("dddd, dd-MMM")
                            + " at "
                            + dt.toString ("hh:mm:ss");
                }
            } else {
                if (0 == daysTo) {
                    strDisp = dt.toString ("hh:mm");
                } else if (1 == daysTo) {
                    strDisp = dt.toString ("hh:mm") + "\nyesterday";
                } else if (daysTo < currentDate.dayOfWeek ()) {
                    strDisp = dt.toString ("hh:mm\ndddd");
                } else {
                    strDisp = dt.toString ("hh:mm:ss") + "\n"
                            + dt.toString ("dd-MMM");
                }
            }

            var = strDisp;
        }
        else if (3 == column)   // GV_IN_DISPNUM
        {
            QString strNum = var.toString ();
            var.clear ();
            if (0 == strNum.size ()) {
                qWarning ("Inbox: Friendly number is blank in entry");
                break;
            }

            if (!GVApi::isNumberValid (strNum)) {
                Q_WARN("Inbox: Display phone number is invalid : ") << strNum;
                var = "Unknown";
                break;
            }

            QString strSimplified = strNum;
            GVApi::simplify_number (strSimplified, false);
            GVApi::simplify_number (strNum);

            CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
            ContactInfo info;
            if (dbMain.getContactFromNumber (strSimplified, info)) {
                var = info.strTitle;
            } else {
                var = strNum;
            }
        }
        else if (4 == column)   // GV_IN_PHONE
        {
            QString strNum = var.toString ();
            var.clear ();
            if (0 == strNum.size ()) {
                qWarning ("Inbox: Number is blank in entry");
                break;
            }
            if (strNum.startsWith ("Unknown")) {
                var = "Unknown";
            }
            if (!GVApi::isNumberValid (strNum)) {
                qWarning () << "Inbox: Actual phone number is invalid : "
                            << strNum;
                break;
            }

            GVApi::beautify_number (strNum);
            var = strNum;
        }
        else if (5 == column)   // GV_IN_FLAGS
        {
            if (IN_ReadFlag == role) {
                var = QVariant(bool(var.toInt() & INBOX_ENTRY_READ_MASK ?
                                        true : false));
            } else {
                var.clear ();
            }
        }
        else if (6 == column)   // GV_IN_SMSTEXT
        {
#define GPLUS_LINK1 "See more: "
#define GPLUS_LINK2 "http://goo.gl/"
#define GPLUS_LINK GPLUS_LINK1 GPLUS_LINK2
            QString strText = var.toString();
            int pos = strText.indexOf (GPLUS_LINK);
            if (pos == -1) {
                // Return the data as is
                break;
            }

            int linkend = pos + sizeof(GPLUS_LINK) - 1;
            QString link = strText.mid(linkend);
            QString rem;

            linkend = link.indexOf(QRegExp("\\s+"));
            if (linkend == -1) {
                // No spaces found... End of text = end of link
            } else {
                link = link.mid (0, linkend);
                rem = strText.mid (pos + sizeof(GPLUS_LINK)-1 + linkend);
            }
            link = QString("<a href=http://goo.gl/%1>See more.</a>").arg(link);

            strText = strText.mid (0, pos) + link + rem;
            var = strText;
        }
        else
        {
            var.clear ();
            qWarning () << "Invalid data column requested in Inbox view: "
                        << column;
        }
    } while (0); // End cleanup block (not a loop)

    return (var);
}//InboxModel::data

QString
InboxModel::type_to_string (GVI_Entry_Type Type)
{
    QString strReturn;
    switch (Type)
    {
    case GVIE_Placed:
        strReturn = "Placed";
        break;
    case GVIE_Received:
        strReturn = "Received";
        break;
    case GVIE_Missed:
        strReturn = "Missed";
        break;
    case GVIE_Voicemail:
        strReturn = "Voicemail";
        break;
    case GVIE_TextMessage:
        strReturn = "SMS";
        break;
    default:
        break;
    }
    return (strReturn);
}//InboxModel::type_to_string

GVI_Entry_Type
InboxModel::string_to_type (const QString &strType)
{
    GVI_Entry_Type Type = GVIE_Unknown;

    do // Begin cleanup block (not a loop)
    {
        if (0 == strType.compare ("Placed", Qt::CaseInsensitive))
        {
            Type = GVIE_Placed;
            break;
        }
        if (0 == strType.compare ("Received", Qt::CaseInsensitive))
        {
            Type = GVIE_Received;
            break;
        }
        if (0 == strType.compare ("Missed", Qt::CaseInsensitive))
        {
            Type = GVIE_Missed;
            break;
        }
        if (0 == strType.compare ("Voicemail", Qt::CaseInsensitive))
        {
            Type = GVIE_Voicemail;
            break;
        }
        if (0 == strType.compare ("SMS", Qt::CaseInsensitive))
        {
            Type = GVIE_TextMessage;
            break;
        }
    } while (0); // End cleanup block (not a loop)

    return (Type);
}//InboxModel::string_to_type

bool
InboxModel::searchById(const QString &id, quint32 &foundRow)
{
    QModelIndex startIndex = this->index (0, 0);
    QModelIndexList foundList = match (startIndex, Qt::EditRole, id, 1,
                                       Qt::MatchExactly);
    bool found = false;
    if (foundList.count () != 0) {
        QModelIndex foundIndex = foundList.at (0);
        foundRow = foundIndex.row ();
        found = true;
    }

    return (found);
}//InboxModel::searchById

bool
InboxModel::refresh (const QString &strSelected)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    if (strSelected != strSelectType)
    {
        strSelectType = strSelected;
        eSelectType = string_to_type (strSelected);
    }

    dbMain.refreshInboxModel (this, strSelected);

    while (this->canFetchMore ()) {
        this->fetchMore ();
    }

    return (true);
}//InboxModel::refresh

bool
InboxModel::refresh ()
{
    return this->refresh (strSelectType);
}//InboxModel::refresh

bool
InboxModel::insertEntry (const GVInboxEntry &hEvent)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    quint32 rowCount = this->rowCount ();

    bool bExists = dbMain.existsInboxEntry (hEvent);

    if (!bExists) {
        beginInsertRows (QModelIndex(), rowCount, rowCount);
    }

    dbMain.insertInboxEntry (hEvent);

    if (!bExists) {
        endInsertRows ();
    } else {
        //emit dataChanged ();
    }

    return (true);
}//InboxModel::insertEntry

bool
InboxModel::deleteEntry (const GVInboxEntry &hEvent)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    bool bExists = dbMain.existsInboxEntry (hEvent);

    if (bExists) {
        quint32 rowToDelete;
        bool found = searchById (hEvent.id, rowToDelete);
        if (found) {
            beginRemoveRows (QModelIndex (), rowToDelete, rowToDelete);
        }
        dbMain.deleteInboxEntryById (hEvent.id);
        if (found) {
            endRemoveRows ();

            this->refresh ();
        }
    }

    return (true);
}//InboxModel::deleteEntry

bool
InboxModel::markAsRead (const QString &msgId)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    GVInboxEntry hEvent;
    hEvent.id = msgId;

    bool bExists = dbMain.existsInboxEntry (hEvent);

    if (bExists) {
        quint32 rowToMark;
        bool found = searchById (hEvent.id, rowToMark);
        //dbMain.markAsRead (msgId);
        if (found) {
            dbMain.markAsRead (msgId);
            QModelIndex foundIndex = index(rowToMark, 5);
            emit dataChanged (foundIndex, foundIndex);

            this->refresh ();
        }
    }

    return (true);
}//InboxModel::markAsRead
