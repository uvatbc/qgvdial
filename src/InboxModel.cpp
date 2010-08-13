#include "InboxModel.h"

InboxModel::InboxModel (QObject * parent, QSqlDatabase db)
: QSqlTableModel (parent, db)
{
}//InboxModel::InboxModel

QVariant
InboxModel::data (const QModelIndex &index,
                        int          role ) const
{
    if ((3 == index.column ()) && (Qt::DisplayRole == role)) // GV_IN_ATTIME
    {
        QVariant var = QSqlTableModel::data (index, role);
        bool bOk = false;
        quint64 num = var.toULongLong (&bOk);
        if (bOk)
        {
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
        }

        return (var);
    }
    return (QSqlTableModel::data (index, role));
}//InboxModel::data
