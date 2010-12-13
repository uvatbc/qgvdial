#ifndef CONTACTSMODEL_H
#define CONTACTSMODEL_H

#include <QtCore>
#include <QtSql>

class ContactsModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    enum ContactsFieldRoles {
        CT_NameRoles = Qt::UserRole + 1,
        CT_ContactsRoles,
    };

    explicit ContactsModel(QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
    QSqlTableModel *modelContacts;
};

#endif // CONTACTSMODEL_H
