#include "global.h"
#include <QDir>

#define DB_NAME ".gvdial.sqlite.db"

QString
get_db_name ()
{
    QString rv = QDir::homePath ();
    if (!rv.endsWith (QDir::separator ()))
    {
        rv += QDir::separator ();
    }
    rv += DB_NAME;

    return (rv);
}//get_db_name
