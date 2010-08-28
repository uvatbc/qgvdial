#ifndef __INBOXMODEL_H__
#define __INBOXMODEL_H__

#include "global.h"
#include <QtSql>

class InboxModel : public QSqlTableModel
{
public:
    InboxModel (QObject * parent = 0, QSqlDatabase db = QSqlDatabase());
    QVariant data (const QModelIndex   &index,
                         int            role = Qt::DisplayRole) const;
    void selectOnly (const QString & filter);

public:
    static QString type_to_string (GVH_Event_Type Type);
    static GVH_Event_Type string_to_type (const QString &strType);
};

#endif //__INBOXMODEL_H__
