#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <QtCore>
#include <QtGui>

#ifdef QT_NO_DEBUG
#define NO_DBGINFO      1
#define POS_BELOW_TAB   1
#else
#define NO_DBGINFO      0
#define POS_BELOW_TAB   2
#endif

// Uncomment these to disable webview even in debug
// #undef NO_DBGINFO
// #define NO_DBGINFO 1

#define SKYPE_CLIENT_NAME "QGVDial"

#define UA_N900   "Mozilla/5.0 (X11; U; Linux armv7l; en-GB; rv:1.9.2a1pre) Gecko/20090928 Firefox/3.5 Maemo Browser 1.4.1.21 RX-51 N900"
#define UA_IPHONE "Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16"

//#define GV_URL              "http://www.google.com/voice"
#define GV_URL          "http://m.google.com/voice"
#define GV_HTTPS        "https://www.google.com/voice"
#define GV_HTTPS_M      "https://www.google.com/voice/m"

#define GV_CLIENTLOGIN "https://www.google.com/accounts/ClientLogin"

struct GVContactNumber
{
    GVContactNumber () : chType ('?') {}

    QString strNumber;
    char    chType;
};

struct GVContactInfo
{
    GVContactInfo () : selected(0) {}

    QString                     strLink;
    QString                     strName;
    QVector<GVContactNumber>    arrPhones;
    int                         selected;
};

struct GVRegisteredNumber
{
    QString     strDisplayName;
    QString     strNumber;
};
typedef QVector<GVRegisteredNumber> GVRegisteredNumberArray;

enum GVH_Event_Type {
    GVHE_Unknown = 0,
    GVHE_Placed,
    GVHE_Received,
    GVHE_Missed,
    GVHE_Voicemail,
    GVHE_TextMessage,
};

struct GVHistoryEvent
{
    GVHistoryEvent ():Type(GVHE_Unknown) {}

    GVH_Event_Type  Type        ;
    QString         strName     ;
    QString         strNameLink ;
    QString         strNumber   ;
    QString         strWhen     ;
    QString         strLink     ;
    QString         strVmail    ;
    QString         strSMS      ;
};
Q_DECLARE_METATYPE (GVHistoryEvent)

QString get_db_name ();

template <class T> class VConv
{
public:
    static T* toPtr(QVariant v)
    {
        return  (T *) v.value<void *>();
    }

    static QVariant toQVariant(T* ptr)
    {
        return qVariantFromValue((void *) ptr);
    }
};

typedef QPair<QString,QString> QStringPair;
typedef QList<QStringPair> QStringPairList;

///////////////////////// Google contacts API related //////////////////////////
enum PhoneType {
    PType_Unknown = 0,
    PType_Mobile,
    PType_Home,
    PType_Other,
};

struct PhoneInfo
{
    PhoneType   Type;
    QString     strNumber;

    void init () {
        Type = PType_Unknown;
        strNumber.clear ();
    }
};
typedef QVector<PhoneInfo> PhoneInfoArray;

struct ContactInfo
{
    //! Unique ID for this contact
    QString         strId;
    //! Contact title a.k.a. the contact name
    QString         strTitle;
    //! Array of phones for this contact
    PhoneInfoArray  arrPhones;

    //! Is this contact deleted?
    bool            bDeleted;

    void init () {
        strId.clear ();
        strTitle.clear ();
        arrPhones.clear ();
        bDeleted = false;
    }
};
////////////////////////////////////////////////////////////////////////////////

#endif //__GLOBAL_H__
