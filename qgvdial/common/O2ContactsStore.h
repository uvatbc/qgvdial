#ifndef O2CONTACTSSTORE_H
#define O2CONTACTSSTORE_H

#include "global.h"
#include "o2abstractstore.h"

class O2ContactsStore : public O2AbstractStore
{
    Q_OBJECT
public:
    explicit O2ContactsStore(QSettings *s, QObject *parent = 0);

    QString value(const QString &key, const QString &defaultValue = QString());
    void setValue(const QString &key, const QString &value);

private:
    QSettings *m_s;
};

#endif // O2CONTACTSSTORE_H
