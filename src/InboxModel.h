#ifndef __INBOXMODEL_H__
#define __INBOXMODEL_H__

#include "global.h"
#include <QtSql>

class InboxModel : public QSqlQueryModel
{
public:
    enum InboxFieldRoles {
        IN_TypeRole = Qt::UserRole + 1,
        IN_TimeRole,
        IN_NameRole,
        IN_NumberRole,
        IN_Link,
        IN_TimeDetail,
        IN_SmsText,
    };

    InboxModel (QObject * parent = 0);
    QVariant data (const QModelIndex   &index,
                         int            role = Qt::DisplayRole) const;

public:
    static QString type_to_string (GVI_Entry_Type Type);
    static GVI_Entry_Type string_to_type (const QString &strType);

    bool refresh (const QString &strSelected);
    bool refresh ();

    bool insertHistory (const GVInboxEntry &hEvent);

private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QString strSelectType;
    GVI_Entry_Type eSelectType;
};

#endif //__INBOXMODEL_H__
