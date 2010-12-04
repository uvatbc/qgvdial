/*
@version: 0.5
@author: Sudheer K. <scifi1947 at gmail.com>
@license: GNU General Public License
*/

#ifndef TELEPATHYUTILITY_H
#define TELEPATHYUTILITY_H

#include <QtCore>
#include <QtDBus>


#define AM_SERVICE          "org.freedesktop.Telepathy.AccountManager"
#define AM_OBJ_PATH         "/org/freedesktop/Telepathy/AccountManager"
#define AM_INTERFACE        "org.freedesktop.Telepathy.AccountManager"
#define DBUS_SERVICE        "org.freedesktop.DBus.Properties"
#define DBUS_OBJ_PATH       "/org/freedesktop/DBus/Properties"
#define DBUS_PROPS_IFACE    "org.freedesktop.DBus.Properties"

namespace org {
namespace freedesktop {
namespace Telepathy {
struct SimplePresence
{
    uint type;
    QString status;
    QString statusMessage;
};
}//Telepathy
}//freedesktop
}//org

Q_DECLARE_METATYPE(org::freedesktop::Telepathy::SimplePresence);

bool operator==(const org::freedesktop::Telepathy::SimplePresence& v1, const org::freedesktop::Telepathy::SimplePresence& v2);
inline bool operator!=(const org::freedesktop::Telepathy::SimplePresence& v1, const org::freedesktop::Telepathy::SimplePresence& v2)
{
    return !operator==(v1, v2);
}
QDBusArgument& operator<<(QDBusArgument& arg, const org::freedesktop::Telepathy::SimplePresence& val);
const QDBusArgument& operator>>(const QDBusArgument& arg, org::freedesktop::Telepathy::SimplePresence& val);

class TelepathyUtility : public QObject
{
    Q_OBJECT
public:
    TelepathyUtility(QObject *parent = 0);
    ~TelepathyUtility();
    QList<QDBusObjectPath> getAllAccounts();
    QString getAccountStatus();
    bool accountExists();
    bool createAccount();
    bool deleteAccount();
};

#endif // TELEPATHYUTILITY_H
