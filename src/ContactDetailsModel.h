#ifndef CONTACTDETAILSMODEL_H
#define CONTACTDETAILSMODEL_H

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class ContactDetailsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ rowCount NOTIFY sigRowcount)

signals:
    //! Never going to be used!
    void sigRowcount();

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
    GVContactInfo info;
};

#endif // CONTACTDETAILSMODEL_H
