/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

#ifndef __CACHEDATABASE_INT_H__
#define __CACHEDATABASE_INT_H__

#define QGVDIAL_DB_NAME     "qgvdial.sqlite.db"
#define QGVDIAL_INI_NAME    "qgvdial.ini"

//////////////////////////////// Settings table ////////////////////////////////
#define GV_S_VAR_USER           "user"
#define GV_S_VAR_PASS           "password"
#define GV_S_VAR_CALLBACK       "callback"
#define GV_S_VAR_INBOX_SEL      "inbox_sel"
#define GV_S_VAR_PIN            "gvpin"
#define GV_S_VAR_PIN_ENABLE     "gvpin_enable"

#define GV_S_VAR_DB_VER         "db_ver"
#define GV_S_VAR_VER            "settings_ver"
////////////////////////////////////////////////////////////////////////////////
// Started using Google Contacts API
// #define GV_S_VALUE_DB_VER   "2010-08-03 11:08:00"
// Registered numbers now include the phone type.
// #define GV_S_VALUE_DB_VER   "2010-08-07 13:48:26"
// All "last updated" fields moved out of settings table into updates table
// #define GV_S_VALUE_DB_VER   "2010-08-13 15:24:15"
// Full text of the sms is now stored in the inbox table.
//#define GV_S_VALUE_DB_VER   "2010-12-30 10:30:03"
// Mosquitto settings changed.
//#define GV_S_VALUE_DB_VER   "2011-02-28 23:41:03"
//Stupidity
//#define GV_S_VALUE_DB_VER   "2011-03-09 14:26:25"
// Added note to inbox and contacts
//#define GV_S_VALUE_DB_VER   "2011-04-04 16:30:00"
// Voicemail now has transcription
//#define GV_S_VALUE_DB_VER   "2011-04-08 17:30:00"
// Encryption introduced into qgvdial. Only for username and password
//#define GV_S_VALUE_DB_VER   "2011-04-19 23:51:00"
// Started using settings ini
//#define GV_S_VALUE_DB_VER   "2011-05-03 11:03:50"
// SMS Text became rich text
//#define GV_S_VALUE_DB_VER   "2011-05-27 14:14:40"
// Photos in contacts
//#define GV_S_VALUE_DB_VER   "2011-07-09 18:58:53"
// Update date time in contacts
//#define GV_S_VALUE_DB_VER   "2011-08-05 12:05:58"
// Stopped using webkit for almost everything
//#define GV_S_VALUE_DB_VER   "2011-12-28 23:50:30"
// Database speed improvements
#define GV_S_VALUE_DB_VER   "2012-01-02 00:27:56"
////////////////////////////////////////////////////////////////////////////////
// Started using versioning for the settings
//#define GV_SETTINGS_VER     "2011-05-13 16:33:50"
// There is no longer any need for an updates table.
//#define GV_SETTINGS_VER     "2011-08-05 12:34:02"
// Log levels need to be at 5 by default
#define GV_SETTINGS_VER     "2012-02-08 23:10:19"
////////////////////////////// GV Contacts table ///////////////////////////////
#define GV_CONTACTS_TABLE   "gvcontacts"
#define GV_C_ID             "id"
#define GV_C_NAME           "name"
#define GV_C_NOTES          "notes"
#define GV_C_PICLINK        "piclink"
#define GV_C_UPDATED        "updated"
/////////////////////////////// GV links table /////////////////////////////////
#define GV_LINKS_TABLE      "gvlinks"
#define GV_L_LINK           "link"
#define GV_L_TYPE           "data_type"
#define GV_L_DATA           "data"

#define GV_L_TYPE_NUMBER    "contact number"
///////////////////////// GV registered numbers table //////////////////////////
#define GV_REG_NUMS_TABLE   "gvregnumbers"
#define GV_RN_NAME          "name"
#define GV_RN_NUM           "number"
#define GV_RN_TYPE          "type"
//////////////////////////////// GV inbox table ////////////////////////////////
#define GV_INBOX_TABLE      "gvinbox"
#define GV_IN_ID            "id"
#define GV_IN_TYPE          "type"          // voicemail,missed,etc.
#define GV_IN_ATTIME        "happened_at"
#define GV_IN_DISPNUM       "display_number"
#define GV_IN_PHONE         "number"
#define GV_IN_FLAGS         "flags"         // read, starred, etc.
#define GV_IN_SMSTEXT       "smstext"       // Full text of the SMS
#define GV_IN_NOTE          "note"          // Note associated with this entry
////////////////////////// GV proxy information table //////////////////////////
#define GV_PROXY_TABLE      "proxy_information"
#define GV_P_FLAGS          "flags"
#define GV_P_HOST           "host"
#define GV_P_PORT           "port"
#define GV_P_USER           "user"
#define GV_P_PASS           "pass"

#define GV_P_F_ENABLE       (1<<0)
#define GV_P_F_USE_SYSTEM   (1<<1)
#define GV_P_F_NEEDS_AUTH   (1<<2)
/////////////////////////// Mosquitto settings table ///////////////////////////
#define GV_MQ_TABLE         "mosquitto_settings"
#define GV_MQ_ENABLED       "enabled"
#define GV_MQ_HOST          "host"
#define GV_MQ_PORT          "port"
#define GV_MQ_TOPIC         "topic"
////////////////////////////////////////////////////////////////////////////////
//////////////////////////// Temporary files table /////////////////////////////
#define GV_TEMP_TABLE       "temp_files"
#define GV_TT_CTIME         "ctime"
#define GV_TT_LINK          "link"
#define GV_TT_PATH          "path"
/////////////////////////////// Cookie Jar table ///////////////////////////////
#define GV_COOKIEJAR_TABLE  "cookie_jar"
#define GV_CJ_DOMAIN        "domain"
#define GV_CJ_EXPIRATION    "expirationDate"
#define GV_CJ_HTTP_ONLY     "isHttpOnly"
#define GV_CJ_IS_SECURE     "isSecure"
#define GV_CJ_IS_SESSION    "isSession"
#define GV_CJ_NAME          "name"
#define GV_CJ_PATH          "path"
#define GV_CJ_VALUE         "value"
////////////////////////////////////////////////////////////////////////////////
#define GV_DEBUG_TABLE      "debug"
#define GV_S_DBG_FAILDIAL   "AlwaysFailDialing"
////////////////////////////////////////////////////////////////////////////////

#endif //__CACHEDATABASE_INT_H__
