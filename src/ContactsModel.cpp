#include "ContactsModel.h"
#include "Singletons.h"
#include "ContactDetailsModel.h"

ContactsModel::ContactsModel (QObject *parent)
: QSqlQueryModel (parent)
, modelContacts (NULL)
{
    QHash<int, QByteArray> roles;
    roles[CT_NameRole]     = "name";
    roles[CT_NotesRole]    = "notes";
    roles[CT_ContactsRole] = "contacts";
    setRoleNames(roles);
}//ContactsModel::ContactsModel

QVariant
ContactsModel::data (const QModelIndex &index, int role) const
{
    QVariant retVar;

    do { // Begin cleanup block (not a loop)
        if (CT_NameRole == role) {
            retVar =
            QSqlQueryModel::data (index.sibling(index.row(), 1), Qt::EditRole);
            break;
        }
        if (CT_NotesRole == role) {
            retVar =
            QSqlQueryModel::data (index.sibling(index.row(), 2), Qt::EditRole);
            break;
        }

        if (CT_ContactsRole == role) {
            ContactInfo info;
            info.strId = QSqlQueryModel::data (index.sibling(index.row(), 0),
                                               Qt::EditRole)
                                  .toString ();
            if (info.strId.isEmpty ()) {
                qWarning ("This link is empty!");
                break;
            }

            CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
            dbMain.getContactFromLink (info);

            QObject *pNonConst = (QObject *) this;
            ContactDetailsModel *pCdm = new ContactDetailsModel (info,
                                                                 pNonConst);
            if (NULL != pCdm) {
                retVar = qVariantFromValue<QObject *> (pCdm);
            } else {
                qWarning ("Failed to allocate contact detail");
            }

            break;
        }

        qDebug ("This role can be handled by the base class");
        retVar = QSqlQueryModel::data (index, role);
    } while (0); // End cleanup block (not a loop)

    return (retVar);
}//ContactsModel::data

int
ContactsModel::rowCount (const QModelIndex & /*parent*/) const
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    return (dbMain.getContactsCount ());
}//ContactsModel::rowCount

bool
ContactsModel::insertContact (const ContactInfo &contactInfo)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    bool bExists = dbMain.existsContact (contactInfo.strId);
    int oldcount = this->rowCount ();

    if (!bExists) {
        beginInsertRows (QModelIndex (), oldcount, oldcount);
    }

    dbMain.insertContact (contactInfo);

    if (!bExists) {
        endInsertRows ();
    }
    
    return (true);
}//ContactsModel::insertContact

bool
ContactsModel::deleteContact (const ContactInfo &contactInfo)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    int oldcount = this->rowCount ();
    bool bExists = dbMain.existsContact (contactInfo.strId);

    if (bExists) {
        beginRemoveRows (QModelIndex (), oldcount, oldcount);
        dbMain.deleteContact (contactInfo.strId);
        endRemoveRows ();
    }
    
    return (true);
}//ContactsModel::deleteContact

void
ContactsModel::clearAll ()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    int rc = this->rowCount ();

    if (0 != rc) {
        beginRemoveRows (QModelIndex (), 0, rc);
        dbMain.clearContacts ();
        endRemoveRows ();
    }
}//ContactsModel::clearAll

void
ContactsModel::refresh ()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    beginResetModel ();
    dbMain.refreshContactsModel (this);
    endResetModel ();

    while (this->canFetchMore ()) {
        this->fetchMore ();
    }
}//ContactsModel::refresh
