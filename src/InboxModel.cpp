#include "InboxModel.h"
#include "Singletons.h"
#include "GVAccess.h"

#define GV_IN_TYPE          "type"          // voicemail,missed,etc.

InboxModel::InboxModel (QObject * parent)
: QSqlQueryModel (parent)
{
}//InboxModel::InboxModel

QVariant
InboxModel::data (const QModelIndex &index,
                        int          role ) const
{
    QVariant var = QSqlQueryModel::data (index, role);

    do // Begin cleanup block (not a loop)
    {
        if (Qt::DisplayRole != role)
        {
            break;
        }

        int column = index.column ();

        if (1 == column)    // GV_IN_TYPE
        {
            char chType = var.toChar().toAscii ();
            QString strDisp = type_to_string ((GVH_Event_Type) chType);
            if (0 == strDisp.size ()) break;

            var = strDisp;
            break;
        }

        if (2 == column)    // GV_IN_ATTIME
        {
            bool bOk = false;
            quint64 num = var.toULongLong (&bOk);
            if (!bOk) break;

            QDateTime dt = QDateTime::fromTime_t (num);
            QString strDisp;
            if (0 == dt.daysTo (QDateTime::currentDateTime ()))
            {
                strDisp = dt.toString ("hh:mm:ss");
            }
            else
            {
                strDisp = dt.toString ("hh:mm:ss on dd-MMM");
            }

            var = strDisp;
            break;
        }

        if (3 == column)    // GV_IN_DISPNUM
        {
            QString strNum = var.toString ();
            if (0 == strNum.size ()) break;
            if (!GVAccess::isNumberValid (strNum)) break;

            GVAccess::simplify_number (strNum);

            CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
            GVContactInfo info;
            if (dbMain.getContactFromNumber (strNum, info))
            {
                var = info.strName;
                break;
            }

            var = "Unknown";
            break;
        }

        if (4 == column)    // GV_IN_PHONE
        {
            QString strNum = var.toString ();
            if (0 == strNum.size ()) break;
            if (strNum.startsWith ("Unknown")) {
                var = "Unknown";
            }
            if (!GVAccess::isNumberValid (strNum)) break;

            GVAccess::beautify_number (strNum);
            var = strNum;

            break;
        }
    } while (0); // End cleanup block (not a loop)

    return (var);
}//InboxModel::data

QString
InboxModel::type_to_string (GVH_Event_Type Type)
{
    QString strReturn;
    switch (Type)
    {
    case GVHE_Placed:
        strReturn = "Placed";
        break;
    case GVHE_Received:
        strReturn = "Received";
        break;
    case GVHE_Missed:
        strReturn = "Missed";
        break;
    case GVHE_Voicemail:
        strReturn = "Voicemail";
        break;
    case GVHE_TextMessage:
        strReturn = "SMS";
        break;
    default:
        break;
    }
    return (strReturn);
}//InboxModel::type_to_string

GVH_Event_Type
InboxModel::string_to_type (const QString &strType)
{
    GVH_Event_Type Type = GVHE_Unknown;

    do // Begin cleanup block (not a loop)
    {
        if (0 == strType.compare ("Placed", Qt::CaseInsensitive))
        {
            Type = GVHE_Placed;
            break;
        }
        if (0 == strType.compare ("Received", Qt::CaseInsensitive))
        {
            Type = GVHE_Received;
            break;
        }
        if (0 == strType.compare ("Missed", Qt::CaseInsensitive))
        {
            Type = GVHE_Missed;
            break;
        }
        if (0 == strType.compare ("Voicemail", Qt::CaseInsensitive))
        {
            Type = GVHE_Voicemail;
            break;
        }
        if (0 == strType.compare ("SMS", Qt::CaseInsensitive))
        {
            Type = GVHE_TextMessage;
            break;
        }
    } while (0); // End cleanup block (not a loop)

    return (Type);
}//InboxModel::string_to_type

//void
//InboxModel::selectOnly (const QString & filter)
//{
//    GVH_Event_Type type = string_to_type (filter);

//    QString strFilter;
//    if (GVHE_Unknown != type)
//    {
//        strFilter = QString(GV_IN_TYPE "='%1'").arg (type);
//    }
//    this->setFilter (strFilter);
//}//InboxModel::selectOnly
