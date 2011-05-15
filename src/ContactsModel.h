#ifndef CONTACTSMODEL_H
#define CONTACTSMODEL_H

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class ContactsModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    enum ContactsFieldRoles {
        CT_NameRole = Qt::UserRole + 1,
        CT_NotesRole,
        CT_ContactsRole,
    };

    explicit ContactsModel(QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool insertContact (const ContactInfo &contactInfo);
    bool deleteContact (const ContactInfo &contactInfo);

    void clearAll ();
    void refresh (const QString &query);
    void refresh ();

private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

private:
    QSqlTableModel *modelContacts;
    QString         strSearchQuery;
};

#endif // CONTACTSMODEL_H
