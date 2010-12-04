/*
@version: 0.5
@author: Sudheer K. <scifi1947 at gmail.com>
@license: GNU General Public License
*/

#include "telepathyutility.h"
#include "accountmanagerproxy.h"
#include "accountproxy.h"
#include "accountcompatproxy.h"
#include <QtCore>
#include <QtDBus>

using namespace org::freedesktop::Telepathy;

TelepathyUtility::TelepathyUtility(QObject *parent)
: QObject(parent)
{
}

TelepathyUtility::~TelepathyUtility()
{
}

//Get a list of all Telepathy accounts
QList<QDBusObjectPath> TelepathyUtility::getAllAccounts()
{
    QList<QDBusObjectPath> objPathList;

    QDBusInterface *iface = new QDBusInterface(AM_SERVICE,AM_OBJ_PATH,DBUS_PROPS_IFACE,QDBusConnection::sessionBus(),this);
    if (iface->isValid()){
        QDBusReply<QDBusVariant> reply = iface->call(QDBus::AutoDetect,"Get",AM_INTERFACE,"ValidAccounts");

        if (reply.isValid()){;
            QDBusVariant validAccounts = reply.value();
            const QVariant var = validAccounts.variant();
            const QDBusArgument arg = var.value<QDBusArgument>();

            arg.beginArray();
            while (!arg.atEnd()){
                QDBusObjectPath opath;
                arg >> opath;
                if (opath.path().contains("qgv")){
                    qDebug() << opath.path();
                }
                objPathList.append(opath);
            }
            arg.endArray();
        }
        else{
            qDebug() << "Error occurred while fetching accounts list "<<reply.error();
        }
    } else {
        qDebug("Error occurred while connecting to DBus interface");
    }

    return (objPathList);
}

//Check if Vicar telepathy account exists
bool TelepathyUtility::accountExists()
{
    bool vicarAccountExists = false;
    QList<QDBusObjectPath> accountsList = this->getAllAccounts();
    QDBusObjectPath account;
    foreach (account,accountsList){
        if (account.path().contains("qgvtp/qgv/qgvtp")){
            vicarAccountExists = true;
            break;
        }
    }

    return vicarAccountExists;
}

//Get telepathy account status
QString TelepathyUtility::getAccountStatus()
{
    QString status = "Not Available";

    QList<QDBusObjectPath> accountsList = this->getAllAccounts();
    QDBusObjectPath account;
    foreach (account,accountsList){
        if (account.path().contains("qgvtp/qgv/qgvtp")){
            AccountProxy *accountProxy = new AccountProxy(AM_SERVICE,account.path(),QDBusConnection::sessionBus(),this);
            if (accountProxy->isValid())
            {
                uint intStatus = accountProxy->property("ConnectionStatus").toUInt();
                //Based on http://telepathy.freedesktop.org/spec/Connection.html#Connection_Status
                switch (intStatus) {
                case 0:
                    status = "Connected";
                    break;
                case 1:
                    status = "Connecting";
                    break;
                case 2:
                    status = "Disconnected";
                    break;
                }
            }
        }
    }

    return status;
}

//Create Vicar telepathy account (used installation)
bool TelepathyUtility::createAccount()
{
    AccountManagerProxy *amProxy = new AccountManagerProxy(AM_SERVICE,AM_OBJ_PATH,QDBusConnection::sessionBus(),this);

    QMap<QString,QVariant> connectionParametersMap;
    connectionParametersMap.insert("account","qgvtp");

    QList<QVariant> presenceDetails;
    uint presenceType(2); //Available = 2
    presenceDetails << presenceType;
    presenceDetails << "online";
    presenceDetails << "Available";

    SimplePresence presence;
    presence.type = presenceType;
    presence.status = "online";
    presence.statusMessage = "Available";

    QMap<QString,QVariant> accountPropertiesMap;
    accountPropertiesMap.insert("org.freedesktop.Telepathy.Account.AutomaticPresence",QVariant::fromValue(presence));
    accountPropertiesMap.insert("org.freedesktop.Telepathy.Account.Enabled",true);
    accountPropertiesMap.insert("org.freedesktop.Telepathy.Account.ConnectAutomatically",true);
    accountPropertiesMap.insert("org.freedesktop.Telepathy.Account.RequestedPresence",QVariant::fromValue(presence));
    accountPropertiesMap.insert("com.nokia.Account.Interface.Compat.Profile","qgvtp");

    QStringList valuesList;
    valuesList.append("TEL");
    accountPropertiesMap.insert("com.nokia.Account.Interface.Compat.SecondaryVCardFields",valuesList);

    QDBusPendingReply<QDBusObjectPath> reply = amProxy->CreateAccount("qgvtp","qgv","qgvtp",connectionParametersMap,accountPropertiesMap);
    reply.waitForFinished();

    if (reply.isValid()){
        QDBusObjectPath account = reply.value();
        qDebug() << account.path() << "created successfully.";

        AccountCompatProxy *accountCompatProxy = new AccountCompatProxy(AM_SERVICE,account.path(),QDBusConnection::sessionBus(),this);
        if (accountCompatProxy->isValid()){
            QDBusPendingReply<> dbusReply = accountCompatProxy->SetHasBeenOnline();
            dbusReply.waitForFinished();
            if (dbusReply.isError()){
                qDebug() << "Error occurred while setting HasBeenOnline property " << dbusReply.error();
                return false;
            }
        }
    }
    else{
        qDebug() << "Error creating VICaR telepathy account " << reply.error();
        return false;
    }

    return true;
}

//Delete Vicar telepathy account (used during uninstallation)
bool TelepathyUtility::deleteAccount()
{
    QList<QDBusObjectPath> accountsList = this->getAllAccounts();
    QDBusObjectPath account;
    foreach (account,accountsList){
        if (account.path().contains("qgvtp/qgv/qgvtp")){
            AccountProxy *accountProxy = new AccountProxy(AM_SERVICE,account.path(),QDBusConnection::sessionBus(),this);
            if (accountProxy->isValid()){
                QDBusPendingReply<> dbusReply = accountProxy->Remove();
                dbusReply.waitForFinished();
                if (dbusReply.isError()){
                    qDebug() << "Error occurred while removing qgv-tp account "<<dbusReply.error();
                    return false;
                } else {
                    qDebug ("qgv-tp account deleted");
                }
            }
        }
    }

    return true;
}

// Marshall the Presence data into a D-Bus argument
QDBusArgument &operator<<(QDBusArgument &argument, const SimplePresence &simplePresence)
{
    argument.beginStructure();
    argument <<  simplePresence.type << simplePresence.status << simplePresence.statusMessage;
    argument.endStructure();
    return argument;
}

// Retrieve the Presence data from the D-Bus argument
const QDBusArgument &operator>>(const QDBusArgument &argument, SimplePresence &simplePresence)
{
    argument.beginStructure();
    argument >> simplePresence.type >> simplePresence.status >> simplePresence.statusMessage;
    argument.endStructure();
    return argument;
}

