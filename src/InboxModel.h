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
};

#endif //__INBOXMODEL_H__