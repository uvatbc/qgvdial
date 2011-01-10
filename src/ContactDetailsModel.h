#ifndef CONTACTDETAILSMODEL_H
#define CONTACTDETAILSMODEL_H

#include "global.h"

class ContactDetailsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ContactDetailRoles {
        CD_TypeRole = Qt::UserRole + 1,
        CD_NumberRole,
    };

    explicit ContactDetailsModel (const GVContactInfo &i, QObject *parent = 0);
    ~ContactDetailsModel ();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool getAt (int index, GVContactNumber &data) const;

private:
    GVContactInfo   info;
};

#endif // CONTACTDETAILSMODEL_H
