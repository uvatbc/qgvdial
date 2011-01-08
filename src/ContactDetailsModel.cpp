#include "ContactDetailsModel.h"
#include "GVAccess.h"

static quint32 nChildren = 0;

ContactDetailsModel::ContactDetailsModel (const GVContactInfo &i,
                                          QObject *parent)
: QAbstractListModel (parent)
, info (i)
{
    nChildren++;
    qDebug () << nChildren << ": ContactDetailsModel constructor" << (void *)this;

    if (nChildren == 256) {
        qDebug ("This is the point after which it usually crashes");
    }

    QHash<int, QByteArray> roles;
    roles[CD_TypeRole]   = "type";
    roles[CD_NumberRole] = "number";
    setRoleNames(roles);
}//ContactDetailsModel::ContactDetailsModel

ContactDetailsModel::~ContactDetailsModel ()
{
    qDebug () << nChildren << ": ContactDetailsModel destructor" << (void *)this;
    nChildren--;
}

int
ContactDetailsModel::rowCount (const QModelIndex & /*parent*/) const
{
    return (info.arrPhones.count ());
}//ContactDetailsModel::rowCount

QVariant
ContactDetailsModel::data (const QModelIndex &index, int role) const
{
    QVariant var;

    do { // Begin cleanup block (not a loop)
        int row = index.row();
        GVContactNumber data;
        if (!getAt (row, data)) {
            break;
        }

        if (CD_TypeRole == role) {
            switch (data.chType) {
            case 'M':
                var = "Mobile";
                break;
            case 'H':
                var = "Home";
                break;
            case 'W':
                var = "Work";
                break;
            default:
            case 'O':
                var = "Other";
                break;
            }

            break;
        }

        if (CD_NumberRole == role) {
            GVAccess::beautify_number (data.strNumber);
            var = data.strNumber;
            break;
        }

    } while (0); // End cleanup block (not a loop)

    return (var);
}//ContactDetailsModel::data

bool
ContactDetailsModel::getAt (int index, GVContactNumber &data) const
{
    bool rv = false;
    do { // Begin cleanup block (not a loop)
        if ((index < 0) || (index >= rowCount ())) {
            qWarning ("Requested an index out of range");
            break;
        }

        if (0 == rowCount ()) {
            qWarning ("List is empty");
            break;
        }

        data = info.arrPhones.at (index);

        rv = true;
    } while (0); // End cleanup block (not a loop)
    return (rv);
}//ContactDetailsModel::getAt
