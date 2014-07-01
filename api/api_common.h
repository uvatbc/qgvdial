/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

#ifndef GAPI_COMMON_H
#define GAPI_COMMON_H

#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include <QtScript>

#include "AsyncTaskToken.h"
#include "NwReqTracker.h"

#define COUNT_OF(x) (sizeof(x)/sizeof(0[x]))

#define UA_N900    "Mozilla/5.0 (X11; U; Linux armv7l; en-GB; rv:1.9.2a1pre) Gecko/20090928 Firefox/3.5 Maemo Browser 1.4.1.21 RX-51 N900"
#define UA_IPHONE  "Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16"
#define UA_IPHONE4 "Mozilla/5.0 (iPhone; U; CPU iPhone OS 4_3_1 like Mac OS X; zh-tw) AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8G4 Safari/6533.18.5"
#define UA_IPOD    "Mozilla/5.0 (iPod; U; CPU iPhone OS 4_3_3 like Mac OS X; ja-jp) AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8J2 Safari/6533.18.5"
#define UA_DESKTOP "Mozilla/5.0 (X11; Linux x86_64; rv:5.0) Gecko/20100101 Firefox/5.0"

#define GV_HTTP         "http://www.google.com/voice"
#define GV_HTTPS        "https://www.google.com/voice"
#define GV_HTTPS_M      "https://www.google.com/voice/m"

#define GOOGLE_ACCOUNTS         "https://accounts.google.com"
// "https://accounts.google.com/ServiceLogin"
#define GV_ACCOUNT_SERVICELOGIN GOOGLE_ACCOUNTS "/ServiceLogin"
// "https://accounts.google.com/SmsAuth"
#define GV_ACCOUNT_SMSAUTH      GOOGLE_ACCOUNTS "/SmsAuth"
// "https://accounts.google.com/ClientLogin"
#define GV_CLIENTLOGIN          GOOGLE_ACCOUNTS "/ClientLogin"

#define POST_FORM "application/x-www-form-urlencoded"
#define POST_TEXT "text/plain"
#define POST_JSON "application/json"

typedef QList<QVariantMap> VarMapList;

struct GVRegisteredNumber
{
    QString     id;
    QString     name;
    QString     number;
    char        chType;

    quint64     active              : 1;
    quint64     verified            : 1;
    quint64     inVerification      : 1;
    quint64     reverifyNeeded      : 1;
    quint64     smsEnabled          : 1;
    quint64     telephonyVerified   : 1;
    quint64     dialBack            : 1;

    QString     forwardingCountry;
    QString     displayUnverifyScheduledDateTime;

    GVRegisteredNumber () {init();}
    void init() {
        chType = 0;
        id.clear ();
        name.clear ();
        number.clear ();
        active = verified = inVerification = reverifyNeeded = smsEnabled
               = telephonyVerified = 0;
        dialBack = 1;
        forwardingCountry.clear ();
        displayUnverifyScheduledDateTime.clear ();
    }
};
typedef QVector<GVRegisteredNumber> GVRegisteredNumberArray;

enum GVI_Entry_Type {
    GVIE_Unknown = 0,
    GVIE_Placed,
    GVIE_Received,
    GVIE_Missed,
    GVIE_Voicemail,
    GVIE_TextMessage
};

struct ConversationEntry {
    QString from;
    QString time;
    QString text;

    void init() {
        from.clear ();
        time.clear ();
        text.clear ();
    }
};

enum GVI_VMailFmt {
    GVIVFMT_Unknown = 0,
    GVIVFMT_Mp3,
    GVIVFMT_Ogg
};

struct GVInboxEntry
{
    //! Unique ID per inbox entry.
    QString         id;
    //! SMS / Call / Voicemail
    GVI_Entry_Type  Type;
    //! The start date and time of this entry.
    QDateTime       startTime;

    QString         strDisplayNumber;
    QString         strPhoneNumber;

    //! The text if this is an SMS
    QString         strText;
    //! The entire conversation
    QVector<ConversationEntry> conversation;
    //! The note associated with this inbox entry, if any.
    QString         strNote;
    //! The duration of the voice mail (if it's a voicemail)
    quint32         vmailDuration;

    GVI_VMailFmt    vmailFormat;

    bool            bRead;
    bool            bSpam;
    bool            bTrash;
    bool            bStar;

    GVInboxEntry () {
        init ();
    }

    void init () {
        id.clear ();
        Type = GVIE_Unknown;
        startTime = QDateTime();
        strDisplayNumber.clear ();
        strPhoneNumber.clear ();
        strText.clear ();
        conversation.clear ();
        strNote.clear ();

        vmailDuration = 0;

        bRead = bSpam = bTrash = bStar = false;
    }
};
Q_DECLARE_METATYPE (GVInboxEntry)

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
    PType_Work,
    PType_Pager,
    PType_Other
};

struct PhoneInfo
{
    PhoneType   Type;
    QString     strNumber;

    PhoneInfo() { init(); }

    void init () {
        Type = PType_Unknown;
        strNumber.clear ();
    }

    static PhoneType charToType(const char ch) {
        switch (ch) {
        case 'M':
            return PType_Mobile;
        case 'H':
            return PType_Home;
        case 'W':
            return PType_Work;
        case 'P':
            return PType_Pager;
        case 'O':
            return PType_Other;
        default:
            return PType_Unknown;
        }
    }
    static char typeToChar (PhoneType type) {
        switch(type) {
        case PType_Mobile:
            return 'M';
        case PType_Home:
            return 'H';
        case PType_Work:
            return 'W';
        case PType_Pager:
            return 'P';
        case PType_Other:
            return 'O';
        default:
            return '?';
        }
    }
    static const char * typeToString (PhoneType type) {
        switch(type) {
        case PType_Mobile:
            return "Mobile";
        case PType_Home:
            return "Home";
        case PType_Work:
            return "Work";
        case PType_Pager:
            return "Pager";
        case PType_Other:
            return "Other";
        default:
            return "Unknown";
        }
    }
};
typedef QVector<PhoneInfo> PhoneInfoArray;

enum EmailType {
    EType_Unknown = 0,
    EType_Home,
    EType_Work,
    EType_Other
};

struct EmailInfo {
    EmailType type;
    QString   address;
    bool      primary;

    EmailInfo() { init(); }
    void init() {
        type = EType_Unknown;
        address.clear ();
        primary = false;
    }
};
typedef QVector<EmailInfo> EmailInfoArray;

enum PostalType {
    PAType_Unknown = 0,
    PAType_Home,
    PAType_Work,
    PAType_Other
};

struct PostalInfo {
    PostalType type;
    QString    address;

    PostalInfo() { init(); }
    void init() {
        type = PAType_Unknown;
        address.clear ();
    }
};
typedef QVector<PostalInfo> PostalInfoArray;

struct ContactInfo
{
    //! Unique ID for this contact
    QString         strId;
    //! Contact title a.k.a. the contact name
    QString         strTitle;

    //! Array of phones for this contact
    PhoneInfoArray  arrPhones;
    //! Which number is selected
    int             selected;

    //! Array of email addresses for this contact
    EmailInfoArray  arrEmails;

    //! Array of postal addresses for this contact
    PostalInfoArray arrPostal;

    //! The notes that the user may have added to the contact
    QString         strNotes;

    //! Link to this contact's photo
    QString         hrefPhoto;
    //! Path to the temp file that has this contacts photo
    QString         strPhotoPath;

    //! When was this contact last updated
    QDateTime       dtUpdate;

    //! Is this contact deleted?
    bool            bDeleted;

    ContactInfo() { init(); }

    void init () {
        strId.clear ();
        strTitle.clear ();
        arrPhones.clear ();
        arrEmails.clear ();
        arrPostal.clear ();
        strNotes.clear ();
        hrefPhoto.clear ();
        strPhotoPath.clear();
        bDeleted = false;
        selected = 0;
        dtUpdate = QDateTime();
    }
};
Q_DECLARE_METATYPE(ContactInfo)
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Debugging
////////////////////////////////////////////////////////////////////////////////

#if defined(__GNUC__)
#define __FULLFUNC__ __PRETTY_FUNCTION__
#else
#define __FULLFUNC__ __FUNCTION__
#endif

#define Q_DEBUG(_s) qDebug()   << QString("%1(%2): %3").arg(__FULLFUNC__).arg(__LINE__).arg(_s)
#define Q_WARN(_s) qWarning()  << QString("%1(%2): %3").arg(__FULLFUNC__).arg(__LINE__).arg(_s)
#define Q_CRIT(_s) qCritical() << QString("%1(%2): %3").arg(__FULLFUNC__).arg(__LINE__).arg(_s)

#endif //GAPI_COMMON_H
