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

//#define GV_URL              "http://www.google.com/voice"
#define GV_URL          "http://m.google.com/voice"
#define GV_HTTPS        "https://www.google.com/voice"
#define GV_HTTPS_M      "https://www.google.com/voice/m"

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

#ifdef Q_WS_MAEMO_5
typedef QWidget ChildWindowBase;
#define ChildWindowBase_flags (Qt::Window)
#else
typedef QDialog ChildWindowBase;
#define ChildWindowBase_flags (0)
#endif

#endif //__GLOBAL_H__
