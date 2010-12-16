#include "ContactsModel.h"
#include "Singletons.h"
#include "ContactDetailsModel.h"

ContactsModel::ContactsModel (QObject *parent)
: QSqlQueryModel (parent)
, modelContacts (NULL)
{
    QHash<int, QByteArray> roles;
    roles[CT_NameRole]     = "name";
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

        if (CT_ContactsRole == role) {
            GVContactInfo info;
            info.strLink = QSqlQueryModel::data (index.sibling(index.row(), 0),
                                                 Qt::EditRole)
                            .toString ();
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

    int oldcount = this->rowCount ();
    beginInsertRows (QModelIndex (), oldcount, oldcount);

    GVContactInfo gvContactInfo;
    convert (contactInfo, gvContactInfo);

    dbMain.insertContact (contactInfo.strTitle, contactInfo.strId);
    dbMain.putContactInfo (gvContactInfo);

    endInsertRows ();
    return (true);
}//ContactsModel::insertContact

bool
ContactsModel::deleteContact (const ContactInfo &contactInfo)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    int oldcount = this->rowCount ();
    beginInsertRows (QModelIndex (), oldcount, oldcount);

    dbMain.deleteContact (contactInfo.strId);
    dbMain.deleteContactInfo (contactInfo.strId);

    endInsertRows ();
    return (true);
}//ContactsModel::deleteContact

bool
ContactsModel::convert (const ContactInfo &cInfo, GVContactInfo &gvcInfo)
{
    gvcInfo.strLink = cInfo.strId;
    gvcInfo.strName = cInfo.strTitle;

    foreach (PhoneInfo pInfo, cInfo.arrPhones)
    {
        GVContactNumber gvcn;
        switch (pInfo.Type)
        {
        case PType_Mobile:
            gvcn.chType = 'M';
            break;
        case PType_Home:
            gvcn.chType = 'H';
            break;
        case PType_Other:
            gvcn.chType = 'O';
            break;
        default:
            gvcn.chType = '?';
            break;
        }

        gvcn.strNumber = pInfo.strNumber;

        gvcInfo.arrPhones += gvcn;
    }

    return (true);
}//ContactsModel::convert
