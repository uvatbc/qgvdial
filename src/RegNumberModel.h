#ifndef REGNUMBERMODEL_H
#define REGNUMBERMODEL_H

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

enum RegNumType {
    RNT_Callback,
    RNT_Callout,
};

struct RegNumData {
    RegNumType  type;
    QString     strName;
    QString     strDesc;

    // For Callout initiators
    void       *pCtx;
    // For callbacks
    char        chType;
};
typedef QList<RegNumData> RegNumDataList;

class RegNumberModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum RegNumberRoles {
        TypeRole = Qt::UserRole + 1,
        NameRole,
        DescriptionRole,
    };

    explicit RegNumberModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool insertRow (const QString &strName,
                    const QString &strDesc,
                    void *pContext = NULL);
    bool insertRow (const QString &strName,
                    const QString &strDesc,
                    const char     chType);

    bool getAt (int index, RegNumData &data) const;

    void clear ();

private:
    RegNumDataList listRegNums;
};

#endif // REGNUMBERMODEL_H
