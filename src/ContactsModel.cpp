#include "ContactsModel.h"
#include "Singletons.h"

ContactsModel::ContactsModel (QObject *parent)
: QSqlQueryModel (parent)
, modelContacts (NULL)
{
    QHash<int, QByteArray> roles;
    roles[CT_NameRoles]     = "name";
    roles[CT_ContactsRoles] = "contacts";
    setRoleNames(roles);
}//ContactsModel::ContactsModel

QVariant
ContactsModel::data (const QModelIndex &index, int role) const
{
    QVariant retVar;

    do { // Begin cleanup block (not a loop)
        if (CT_NameRoles == role) {
            retVar =
            QSqlQueryModel::data (index.sibling(index.row(), 1), Qt::EditRole);
            break;
        }

        if (CT_ContactsRoles == role) {
            //@@UV: Need to create another SQL model here
            break;
        }

        qDebug ("This role can be handled by the base class");
        retVar = QSqlQueryModel::data (index, role);
    } while (0); // End cleanup block (not a loop)

    return (retVar);
}//ContactsModel::data
