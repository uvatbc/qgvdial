#include "DBusApi.h"
#include "gen/api_adapter.h"
#include "Singletons.h"

QGVDBusCallApi::QGVDBusCallApi(QObject *parent)
: QObject(parent)
{
}//QGVDBusCallApi::QGVDBusCallApi

QGVDBusCallApi::~QGVDBusCallApi()
{
}//QGVDBusCallApi::~QGVDBusCallApi

bool
QGVDBusCallApi::registerObject()
{
    CallServerAdaptor *obj = new CallServerAdaptor(this);
    if (NULL == obj) {
        return false;
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerObject ("/org/QGVDial/CallServer", this)) {
        delete obj;
        return false;
    }

    return true;
}//QGVDBusCallApi::registerObject

void
QGVDBusCallApi::Call(const QString &strNumber)
{
    Q_DEBUG(QString("Make a call to %1").arg (strNumber));
    emit dialNow (strNumber);
}//QGVDBusCallApi::Call

QGVDBusTextApi::QGVDBusTextApi(QObject *parent)
: QObject(parent)
{
}//QGVDBusTextApi::QGVDBusTextApi

QGVDBusTextApi::~QGVDBusTextApi()
{
}//QGVDBusTextApi::~QGVDBusTextApi

bool
QGVDBusTextApi::registerObject()
{
    TextServerAdaptor *obj = new TextServerAdaptor(this);
    if (NULL == obj) {
        return false;
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerObject ("/org/QGVDial/TextServer", this)) {
        delete obj;
        return false;
    }

    return true;
}//QGVDBusTextApi::registerObject

void
QGVDBusTextApi::Text(const QStringList &arrNumbers, const QString &strData)
{
    Q_DEBUG(QString("Send text \"%1\" to (%2)")
            .arg (strData)
            .arg (arrNumbers.join (", ")));
    emit sendText (arrNumbers, strData);
}//QGVDBusTextApi::Text

void
QGVDBusTextApi::TextWithoutData(const QStringList &arrNumbers)
{
    Q_DEBUG(QString("Send text without data to (%1)")
            .arg (arrNumbers.join (", ")));
    emit sendTextWithoutData (arrNumbers);
}//QGVDBusTextApi::TextWithoutData

QStringList
QGVDBusTextApi::getTextsByContact(const QString &strContact)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QStringList rv = dbMain.getTextsByContact(strContact);

    Q_DEBUG(QString("Got dbus request to get texts from contact matching "
                    "\"%1\". Returning %2 texts")
            .arg(strContact).arg (rv.count ()));

    return rv;
}//QGVDBusTextApi::getTextsByContact

QStringList
QGVDBusTextApi::getTextsByDate(const QString &strStart, const QString &strEnd)
{
    QStringList rv;
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    QDateTime dtStart = QDateTime::fromString(strStart, Qt::ISODate);
    QDateTime dtEnd = QDateTime::fromString(strEnd, Qt::ISODate);

    do { // Begin cleanup block (not a loop)
        if (!dtStart.isValid () || !dtEnd.isValid ()) {
            qWarning ("getTextsByDate: Invalid dates");
            break;
        }

        // If reversed, fix it
        if (dtStart > dtEnd) {
            QDateTime dtTemp = dtEnd;
            dtEnd = dtStart;
            dtStart = dtTemp;
        }

        // Send request
        rv = dbMain.getTextsByDate (dtStart, dtEnd);


        Q_DEBUG(QString("Got dbus request to get texts between %1 and %2. "
                        "Returning %3 texts")
                .arg(strStart, strEnd).arg (rv.count ()));
    } while(0); // End cleanup block (not a loop)

    return rv;
}//QGVDBusTextApi::getTextsByDate

QGVDBusSettingsApi::QGVDBusSettingsApi(QObject *parent)
: QObject(parent)
, m_phoneIndex(-1)
{
}//QGVDBusSettingsApi::QGVDBusSettingsApi

QGVDBusSettingsApi::~QGVDBusSettingsApi()
{
}//QGVDBusSettingsApi::~QGVDBusSettingsApi

bool
QGVDBusSettingsApi::registerObject()
{
    SettingsServerAdaptor *obj = new SettingsServerAdaptor(this);
    if (NULL == obj) {
        return false;
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerObject ("/org/QGVDial/SettingsServer", this)) {
        delete obj;
        return false;
    }

    return true;
}//QGVDBusSettingsApi::registerObject

int
QGVDBusSettingsApi::GetCurrentPhone()
{
    Q_DEBUG("DBus request to get current phone index");
    return m_phoneIndex;
}//QGVDBusSettingsApi::GetCurrentPhone

QStringList
QGVDBusSettingsApi::GetPhoneNames()
{
    Q_DEBUG("DBus request to get phone names");
    return m_phones;
}//QGVDBusSettingsApi::GetPhoneNames

bool
QGVDBusSettingsApi::SetCurrentPhone(int index)
{
    return (emit phoneIndexChange(index));
}//QGVDBusSettingsApi::SetCurrentPhone

void
QGVDBusSettingsApi::onPhoneChanges(const QStringList &phones, int index)
{
    m_phones = phones;
    m_phoneIndex = index;
    emit CallbacksChanged();
}//QGVDBusSettingsApi::onPhoneChanges
