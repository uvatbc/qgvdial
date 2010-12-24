#include "RegNumberModel.h"

RegNumberModel::RegNumberModel (QObject *parent)
: QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[NameRole] = "name";
    roles[DescriptionRole] = "description";
    setRoleNames(roles);
}//RegNumberModel::RegNumberModel

int
RegNumberModel::rowCount (const QModelIndex & /*parent*/) const
{
    return (listRegNums.count ());
}//RegNumberModel::rowCount

QVariant
RegNumberModel::data (const QModelIndex &index, int role) const
{
    QVariant var;
    do { // Begin cleanup block (not a loop)
        int row = index.row();
        RegNumData data;
        if (!getAt (row, data)) {
            break;
        }
        if (TypeRole == role) {
            var = data.type;
            break;
        }
        if (NameRole == role) {
            var = data.strName;
            break;
        }
        if (DescriptionRole == role) {
            var = data.strDesc;
            break;
        }
    } while (0); // End cleanup block (not a loop)
    return (var);
}//RegNumberModel::data

bool
RegNumberModel::insertRow (const QString &strName,
                           const QString &strDesc,
                           void *pContext)
{
    int oldcount = this->rowCount ();
    beginInsertRows (QModelIndex (), oldcount, oldcount);

    RegNumData data;
    data.type    = RNT_Callout;
    data.strName = strName;
    data.strDesc = strDesc;
    data.pCtx    = pContext;

    listRegNums.append (data);
    endInsertRows ();
    return (true);
}//RegNumberModel::insertRow

bool
RegNumberModel::insertRow (const QString &strName,
                           const QString &strDesc,
                           const char     chType)
{
    int oldcount = listRegNums.count ();
    beginInsertRows (QModelIndex (), oldcount, oldcount);

    RegNumData data;
    data.type    = RNT_Callback;
    data.strName = strName;
    data.strDesc = strDesc;
    data.chType  = chType;

    listRegNums.append (data);
    endInsertRows ();
    return (true);
}//RegNumberModel::insertRow

bool
RegNumberModel::getAt (int index, RegNumData &data) const
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

        data = listRegNums.at (index);

        rv = true;
    } while (0); // End cleanup block (not a loop)
    return (rv);
}//RegNumberModel::getRow

void
RegNumberModel::clear ()
{
    listRegNums.clear ();
    this->reset ();
}//RegNumberModel::clear
