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

#include "ContactsParser.h"
#include "ContactsXmlHandler.h"

#define QGC_MEASURE_TIME 0

ContactsParser::ContactsParser (AsyncTaskToken *task, QByteArray data,
                                QObject *parent /* = 0*/)
: QObject(parent)
, m_task (task)
, byData (data)
, bEmitLog (true)
{
}//ContactsParser::ContactsParser

void
ContactsParser::doXmlWork ()
{
    bool rv;
    QXmlInputSource inputSource;
    QXmlSimpleReader simpleReader;
    ContactsXmlHandler contactsHandler;
    contactsHandler.setEmitLog (bEmitLog);

#if QGC_MEASURE_TIME
    QDateTime startTime = QDateTime::currentDateTime ();
#endif

    inputSource.setData (byData);

    rv = connect (&contactsHandler, SIGNAL(status(const QString&,int)),
                  this,             SIGNAL(status(const QString&,int)));
    Q_ASSERT(rv);
    rv = connect (
            &contactsHandler, SIGNAL   (oneContact(ContactInfo)),
            this,             SIGNAL(gotOneContact(ContactInfo)));
    Q_ASSERT(rv);

    simpleReader.setContentHandler (&contactsHandler);
    simpleReader.setErrorHandler (&contactsHandler);

    rv = simpleReader.parse (&inputSource, false);

#if QGC_MEASURE_TIME
    QDateTime endTime = QDateTime::currentDateTime ();
    Q_DEBUG(QString("XML parse took %1 msec")
            .arg(endTime.toMSecsSinceEpoch() - startTime.toMSecsSinceEpoch()));
#endif

    if (!rv) {
        Q_WARN(QString("Contacts parser failed to parse. Data = %1")
               .arg (QString(byData)));
    }

    quint32 total = contactsHandler.getTotalContacts ();
    quint32 usable = contactsHandler.getUsableContacts ();
    emit done(m_task, rv, total, usable);

    if (bEmitLog || (contactsHandler.getUsableContacts () != 0)) {
        QString msg = QString("Total contacts: %1. Usable: %2")
                .arg (total).arg (usable);
        emit status(msg);
    }
}//ContactsParser::doXmlWork

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
void
ContactsParser::doJsonWork ()
{
#if QGC_MEASURE_TIME
    QDateTime startTime = QDateTime::currentDateTime ();
#endif

    QString json = QString(byData);
    QJsonParseError pE;
    QJsonDocument doc = QJsonDocument::fromJson (json.toUtf8 (), &pE);
    quint32 total = 0;

    do {
        if (QJsonParseError::NoError != pE.error) {
            Q_WARN(QString("Failed to parse JSON").arg(json));
            break;
        }

        if (!doc.isObject ()) {
            Q_WARN(QString("JSON is not object").arg(json));
            break;
        }
        QJsonObject jTop = doc.object ();

        if (!jTop.contains ("feed") || !jTop.value("feed").isObject ()) {
            Q_WARN(QString("feed is not an object: %1").arg(json));
            break;
        }
        QJsonObject jFeed = jTop.value("feed").toObject ();

        if (!jFeed.contains ("entry")) {
            Q_WARN("No entries found");
            break;
        }
        if (!jFeed.value("entry").isArray ()) {
            Q_WARN(QString("entry is not an array: %1").arg(json));
            break;
        }
        QJsonArray jEntries = jFeed.value("entry").toArray ();

        total = jEntries.size ();
        ContactInfo ci;

        QJsonArray::iterator it;
        for (it = jEntries.begin(); it != jEntries.end(); ++it) {
            if (!(*it).isObject ()) {
                Q_WARN(QString("entry is not an object: %1").arg(json));
                continue;
            }

            QJsonObject e = (*it).toObject();
            if (!e.contains("id") || !e.value("id").isObject ()) {
                Q_WARN(QString("entry id is not an object: %1").arg(json));
                continue;
            }

            QJsonObject tmp = e.value("id").toObject ();
            if (!tmp.contains("$t") || !tmp.value("$t").isString ()) {
                Q_WARN(QString("entry id.$t is not a string: %1").arg(json));
                continue;
            }

            ci.init ();
            ci.strId = tmp.value ("$t").toString ();

            if (!e.contains("title") || !e.value("title").isObject ()) {
                Q_WARN(QString("entry title is not an object: %1").arg(json));
                continue;
            }

            tmp = e.value("title").toObject ();
            if (!tmp.contains("$t") || !tmp.value("$t").isString ()) {
                Q_WARN(QString("entry title.$t is not a string: %1").arg(json));
                continue;
            }
            ci.strTitle = tmp.value ("$t").toString ();

            if (e.contains ("gd$phoneNumber") &&
                e.value("gd$phoneNumber").isArray ())
            {
                QJsonArray jPArray = e.value("gd$phoneNumber").toArray ();

                PhoneInfo pi;
                QJsonArray::iterator pit;
                int max = jPArray.count ();
                pit = jPArray.begin ();
                for (int i = 0; i < max; i++, pit++) {
                    if (!(*pit).isObject ()) {
                        Q_WARN(QString("phone is not an object: %1").arg(json));
                        continue;
                    }

                    QJsonObject p = (*pit).toObject();
                    if (!p.contains("$t") || !p.value("$t").isString ()) {
                        Q_WARN(QString("phone is not a string: %1").arg(json));
                        continue;
                    }

                    pi.init ();
                    pi.strNumber = p.value ("$t").toString ();

                    QString rStr = p.value ("rel").toString ();
                    if (rStr.contains ("mobile")) {
                        pi.Type = PType_Mobile;
                    } else if (rStr.contains ("home")) {
                        pi.Type = PType_Home;
                    } else if (rStr.contains ("work")) {
                        pi.Type = PType_Work;
                    } else if (rStr.contains ("pager")) {
                        pi.Type = PType_Pager;
                    } else {
                        pi.Type = PType_Other;
                    }
                    ci.arrPhones += pi;
                }
            }

            if (e.contains ("gd$email") &&
                e.value("gd$email").isArray ())
            {
                QJsonArray jArray = e.value("gd$email").toArray ();

                EmailInfo ei;
                QJsonArray::iterator eit;
                int max = jArray.count ();
                eit = jArray.begin ();
                for (int i = 0; i < max; i++, eit++) {
                    if (!(*eit).isObject ()) {
                        Q_WARN(QString("email is not an object: %1").arg(json));
                        continue;
                    }

                    QJsonObject p = (*eit).toObject();
                    if (!p.contains("address") ||
                        !p.value("address").isString ())
                    {
                        Q_WARN(QString("Email is not valid: %1").arg(json));
                        continue;
                    }

                    ei.init ();
                    ei.address = p.value ("address").toString ();

                    if (p.contains("primary")) {
                        if (p.value ("primary").isBool ()) {
                            ei.primary = p.value ("primary").toBool ();
                        } else if (p.value ("primary").isString ()) {
                            ei.primary = p.value("primary").toString()=="true";
                        }
                    }

                    QString rStr = p.value ("rel").toString ();
                    if (rStr.contains ("home")) {
                        ei.type = EType_Home;
                    } else if (rStr.contains ("work")) {
                        ei.type = EType_Work;
                    } else {
                        ei.type = EType_Other;
                    }
                    ci.arrEmails += ei;
                }
            }

            emit gotOneContact (ci);
        }
    } while (0);

#if QGC_MEASURE_TIME
    QDateTime endTime = QDateTime::currentDateTime ();
    Q_DEBUG(QString("JSON parse took %1 msec")
            .arg(endTime.toMSecsSinceEpoch() - startTime.toMSecsSinceEpoch()));
#endif

    emit done(m_task, true, total, total);
}//ContactsParser::doJsonWork
#else
void
ContactsParser::doJsonWork ()
{
#if QGC_MEASURE_TIME
    QDateTime startTime = QDateTime::currentDateTime ();
#endif

    QScriptEngine e;
    QString cmd = QString("var o = %1").arg (QString(byData));
    QString rStr, tmpl;

    quint32 total = 0;

    do {
        e.evaluate (cmd);
        if (e.hasUncaughtException ()) {
            Q_WARN("Failed to evaluate contacts JSON");
            break;
        }

        cmd = "o.feed.entry.length";
        rStr = e.evaluate (cmd).toString ();
        if (e.hasUncaughtException ()) {
            Q_WARN("Failed to evaluate contacts JSON");
            break;
        }

        total = rStr.toUInt ();
        if (0 == total) {
            Q_DEBUG("No contacts present");
            break;
        }

        ContactInfo ci;
        for (quint32 i = 0; i < total; i++) {
            ci.init ();

            cmd = QString("var e = o.feed.entry[%1]").arg (i);
            e.evaluate (cmd);
            if (e.hasUncaughtException ()) {
                Q_WARN("Failed to evaluate contact entry");
                continue;
            }

            cmd = "e.id.$t";
            ci.strId = e.evaluate (cmd).toString ();
            if (e.hasUncaughtException ()) {
                Q_WARN("Failed to evaluate contact id");
                continue;
            }

            cmd = "e.title.$t";
            ci.strTitle = e.evaluate (cmd).toString ();
            if (e.hasUncaughtException ()) {
                Q_WARN("Failed to evaluate contact title");
                continue;
            }

            cmd = "e.gd$phoneNumber.length";
            quint32 max1 = e.evaluate (cmd).toUInt32 ();
            while (!e.hasUncaughtException ()) {
                cmd = "var pi = e.gd$phoneNumber";
                e.evaluate (cmd);
                if (e.hasUncaughtException ()) {
                    Q_WARN("Failed to evaluate contact phone info");
                    break;
                }

                PhoneInfo pi;
                for (quint32 j = 0; j < max1; j++) {
                    pi.init ();
                    tmpl = QString("pi[%1]").arg(j);

                    cmd = tmpl + ".$t";
                    pi.strNumber = e.evaluate (cmd).toString ();
                    if (e.hasUncaughtException ()) {
                        Q_WARN("Failed to evaluate contact phone number");
                        continue;
                    }

                    cmd = tmpl + ".rel";
                    rStr = e.evaluate (cmd).toString ();
                    if (e.hasUncaughtException ()) {
                        Q_WARN("Failed to evaluate contact phone type");
                    }
                    if (rStr.contains ("mobile")) {
                        pi.Type = PType_Mobile;
                    } else if (rStr.contains ("home")) {
                        pi.Type = PType_Home;
                    } else if (rStr.contains ("work")) {
                        pi.Type = PType_Work;
                    } else if (rStr.contains ("pager")) {
                        pi.Type = PType_Pager;
                    } else {
                        pi.Type = PType_Other;
                    }
                    ci.arrPhones += pi;
                }

                break;
            }

            tmpl = QString("o.feed.entry[%1].gd$email").arg (i);
            cmd = tmpl + ".length";
            max1 = e.evaluate (cmd).toUInt32 ();
            if (!e.hasUncaughtException ()) {
                cmd = "var ei = e.gd$email";
                e.evaluate (cmd);
                if (e.hasUncaughtException ()) {
                    Q_WARN("Failed to evaluate contact email info");
                    break;
                }

                EmailInfo ei;
                for (quint32 j = 0; j < max1; j++) {
                    ei.init ();
                    tmpl = QString("ei[%1]").arg(j);

                    cmd = tmpl + ".address";
                    ei.address = e.evaluate (cmd).toString ();
                    if (e.hasUncaughtException ()) {
                        Q_WARN("Failed to evaluate contact email");
                        continue;
                    }

                    cmd = tmpl + ".rel";
                    rStr = e.evaluate (cmd).toString ();
                    if (e.hasUncaughtException ()) {
                        Q_WARN("Failed to evaluate contact email type");
                    }
                    if (rStr.contains ("home")) {
                        ei.type = EType_Home;
                    } else if (rStr.contains ("work")) {
                        ei.type = EType_Work;
                    } else {
                        ei.type = EType_Other;
                    }
                    ci.arrEmails += ei;
                }
            }

            emit gotOneContact (ci);
        }
    } while (0);

#if QGC_MEASURE_TIME
    QDateTime endTime = QDateTime::currentDateTime ();
    Q_DEBUG(QString("JSON parse took %1 msec")
            .arg(endTime.toMSecsSinceEpoch() - startTime.toMSecsSinceEpoch()));
#endif

    emit done(m_task, true, total, total);
}//ContactsParser::doJsonWork
#endif

ContactsParser::~ContactsParser()
{
}//ContactsParser::~ContactsParser

void
ContactsParser::setEmitLog (bool enable /* = true*/)
{
    bEmitLog = enable;
}//ContactsParser::setEmitLog
