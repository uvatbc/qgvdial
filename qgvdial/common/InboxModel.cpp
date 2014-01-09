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

#include "InboxModel.h"
#include "GVApi.h"

#define PIXMAP_SCALED_W 25
#define PIXMAP_SCALED_H 25

InboxModel::InboxModel (QObject * parent)
: QSqlQueryModel (parent)
, strSelectType ("all")
, m_eSelectType (GVIE_Unknown)
{
    QHash<int, QByteArray> roles;
    roles[IN_IdRole]    = "id";
    roles[IN_TypeRole]  = "type";
    roles[IN_NameRole]  = "name";
    roles[IN_NumberRole]= "number";
    roles[IN_TimeRole]  = "time";
    roles[IN_TimeDetail]= "time_detail";
    roles[IN_ReadFlag]  = "is_read";
    setRoleNames(roles);
}//InboxModel::InboxModel

QVariant
InboxModel::data (const QModelIndex &index, int role) const
{
    QVariant var;

    do {
        int column = -1;
        switch (role) {
        case IN_IdRole:
            column = 0;
            break;
        case IN_TypeRole:
            column = 1;
            break;
        case IN_NameRole:
            column = 2;
            break;
        case IN_NumberRole:
            column = 3;
            break;
        case IN_TimeRole:
        case IN_TimeDetail:
            column = 4;
            break;
        case IN_ReadFlag:
            column = 5;
            break;
        case Qt::DisplayRole:
        case Qt::EditRole:
            column = index.column ();
            break;
        case Qt::DecorationRole:
        case Qt::DecorationPropertyRole:
            column = index.column ();
            if (2 != column) {
                column = -1;
            }
            break;
        }

        // Pick up the data from the base class
        var = QSqlQueryModel::data (index.sibling(index.row (), column),
                                    Qt::EditRole);

        if (0 == column) {          // GV_IN_ID
            // No modifications
        } else if (1 == column) {   // GV_IN_TYPE
            char chType = var.toChar().toLatin1 ();
            QString strDisp = type_to_string ((GVI_Entry_Type) chType);
            var.clear ();
            if (0 == strDisp.size ()) {
                Q_WARN(QString("Entry type could not be deciphered: %1")
                       .arg(int(chType)));
                break;
            }

            var = strDisp;
        } else if (2 == column) {   // GV_IN_DISPNUM
            if (Qt::DecorationRole == role) {
                QVariant v;
                v = QSqlQueryModel::data (index.sibling(index.row (), 1), //Type
                                          Qt::EditRole);
                GVI_Entry_Type type = (GVI_Entry_Type) v.toChar().toLatin1();
                QString path;
                switch (type) {
                case GVIE_Placed:
                    path = ":/in_Placed.png";
                    break;
                case GVIE_Received:
                    path = ":/in_Received.png";
                    break;
                case GVIE_Missed:
                    path = ":/in_Missed.png";
                    break;
                case GVIE_Voicemail:
                    path = ":/in_Voicemail.png";
                    break;
                case GVIE_TextMessage:
                    path = ":/in_Sms.png";
                    break;
                default:
                    break;
                }

                var.clear ();

                if (path.isEmpty ()) {
                    break;
                }

    #if !defined(Q_OS_BLACKBERRY)
                QPixmap pixmap(path);
                var = pixmap.scaled(PIXMAP_SCALED_W, PIXMAP_SCALED_H,
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);
    #endif
                break;
            } else if (Qt::DecorationPropertyRole == role) {
                QSize size(PIXMAP_SCALED_W, PIXMAP_SCALED_H);
                var = size;
                break;
            }

            QString strNum = var.toString ();
            var.clear ();
            if (0 == strNum.length()) {
                Q_WARN("Friendly number is blank in entry");
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

            ContactInfo info;
            if (db.getContactFromNumber (strSimplified, info)) {
                var = info.strTitle;
            } else {
                var = strNum;
            }
        } else if (3 == column) {   // GV_IN_PHONE
            QString strNum = var.toString ();
            var.clear ();
            if (0 == strNum.size ()) {
                Q_WARN("Number is blank in entry");
                break;
            }
            if (strNum.startsWith ("Unknown")) {
                var = "Unknown";
            }
            if (!GVApi::isNumberValid (strNum)) {
                Q_WARN(QString("Actual phone number is invalid: %1")
                       .arg (strNum));
                break;
            }

            GVApi::beautify_number (strNum);
            var = strNum;
        } else if (4 == column) {   // GV_IN_ATTIME
            bool ok = false;
            quint64 num = var.toULongLong (&ok);
            var.clear ();
            if (!ok) break;

            QDateTime dt = QDateTime::fromTime_t (num);
            var = dateToString (dt, (IN_TimeDetail == role));
        } else if (5 == column) {   // GV_IN_FLAGS
            if (IN_ReadFlag == role) {
                var = QVariant(bool(var.toInt() & INBOX_ENTRY_READ_MASK ?
                                        true : false));
            } else {
                var.clear ();
            }
        } else {
            var.clear ();
//            Q_WARN(QString("Invalid data column: %1. Actual: %2. Role = %3")
//                   .arg(column).arg(index.column ()).arg(role));
        }
    } while (0);

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

    do {
        if (0 == strType.compare ("Placed", Qt::CaseInsensitive)) {
            Type = GVIE_Placed;
            break;
        }
        if (0 == strType.compare ("Received", Qt::CaseInsensitive)) {
            Type = GVIE_Received;
            break;
        }
        if (0 == strType.compare ("Missed", Qt::CaseInsensitive)) {
            Type = GVIE_Missed;
            break;
        }
        if (0 == strType.compare ("Voicemail", Qt::CaseInsensitive)) {
            Type = GVIE_Voicemail;
            break;
        }
        if (0 == strType.compare ("SMS", Qt::CaseInsensitive)) {
            Type = GVIE_TextMessage;
            break;
        }
    } while (0);

    return (Type);
}//InboxModel::string_to_type

QString
InboxModel::dateToString(QDateTime dt, bool detailed)
{
    QString strDisp;
    QDate currentDate = QDate::currentDate ();
    int daysTo = dt.daysTo (QDateTime::currentDateTime ());
    if (detailed) {
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

    return strDisp;
}//InboxModel::dateToString

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
    GVI_Entry_Type eSelected = GVIE_Unknown;
    if (strSelected != strSelectType) {
        eSelected = string_to_type (strSelected);
    }
    if ((GVIE_Unknown != eSelected) ||
        (0 == strSelected.compare ("all", Qt::CaseInsensitive))) {
        strSelectType = strSelected;
        m_eSelectType = eSelected;
    } else {
        return (false);
    }

    beginResetModel ();
    db.refreshInboxModel (this, strSelected);
    while (this->canFetchMore ()) {
        this->fetchMore ();
    }
    endResetModel ();

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
    quint32 rowCount = this->rowCount ();

    bool bExists = db.existsInboxEntry (hEvent);

    if (!bExists) {
        beginInsertRows (QModelIndex(), rowCount, rowCount);
    }

    db.insertInboxEntry (hEvent);

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
    bool bExists = db.existsInboxEntry (hEvent);

    if (bExists) {
        quint32 rowToDelete;
        bool found = searchById (hEvent.id, rowToDelete);
        if (found) {
            beginRemoveRows (QModelIndex (), rowToDelete, rowToDelete);
        }
        db.deleteInboxEntryById (hEvent.id);
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
    GVInboxEntry hEvent;
    hEvent.id = msgId;

    bool bExists = db.existsInboxEntry (hEvent);

    if (bExists) {
        quint32 rowToMark;
        bool found = searchById (hEvent.id, rowToMark);
        //db.markAsRead (msgId);
        if (found) {
            db.markAsRead (msgId);
            QModelIndex foundIndex = index(rowToMark, 5);
            emit dataChanged (foundIndex, foundIndex);

            this->refresh ();
        }
    }

    return (true);
}//InboxModel::markAsRead
