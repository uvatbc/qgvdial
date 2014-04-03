#include "O2ContactsStore.h"

O2ContactsStore::O2ContactsStore(QSettings *s, QObject *parent)
: O2AbstractStore(parent)
, m_s(s)
{
}//O2ContactsStore::O2ContactsStore

QString
O2ContactsStore::value(const QString &key, const QString &defaultValue)
{
    return m_s->value (key, defaultValue).toString ();
}//O2ContactsStore::value

void
O2ContactsStore::setValue(const QString &key, const QString &value)
{
    m_s->setValue(key, value);
}//O2ContactsStore::setValue
