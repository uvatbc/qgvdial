#ifndef CONTACTSMODEL_H
#define CONTACTSMODEL_H

#include "global.h"
#include <QtSql>

class ContactsModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    enum ContactsFieldRoles {
        CT_NameRole = Qt::UserRole + 1,
        CT_ContactsRole,
    };

    explicit ContactsModel(QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool insertContact (const ContactInfo &contactInfo);
    bool deleteContact (const ContactInfo &contactInfo);

    void clearAll ();
private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    bool convert (const ContactInfo &cInfo, GVContactInfo &gvcInfo);

private:
    QSqlTableModel *modelContacts;
};

#endif // CONTACTSMODEL_H
