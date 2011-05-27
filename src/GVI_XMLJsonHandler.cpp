/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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

#include "GVI_XMLJsonHandler.h"
#include <QtXmlPatterns>

GVI_XMLJsonHandler::GVI_XMLJsonHandler (QObject *parent)
: QObject (parent)
, nUsableMsgs (0)
, bEmitLog (true)
{
}//GVI_XMLJsonHandler::GVI_XMLJsonHandler

bool
GVI_XMLJsonHandler::startElement (const QString        & /*namespaceURI*/,
                                  const QString        & /*localName   */,
                                  const QString        & /*qName       */,
                                  const QXmlAttributes & /*atts        */)
{
    strChars.clear ();
    return (true);
}//GVI_XMLJsonHandler::startElement

bool
GVI_XMLJsonHandler::endElement (const QString & /*namespaceURI*/,
                               const QString &localName   ,
                               const QString & /*qName       */)
{
    if (localName == "json") {
        strJson = strChars;
        if (bEmitLog) qDebug ("Got json characters");

#if 0
        QFile temp("dump.txt");
        temp.open (QIODevice::ReadWrite);
        temp.write (strJson.toAscii ());
        temp.close ();
#endif
    }

    if (localName == "html") {
        strHtml = "<html>"
                + strChars
                + "</html>";
        if (bEmitLog) qDebug ("Got html characters");

#if 0
        QFile temp("dump.txt");
        temp.open (QIODevice::ReadWrite);
        temp.write (strHtml.toAscii ());
        temp.close ();
#endif

        mapTexts.clear ();

        QDomNamedNodeMap attrs;
        QDomDocument doc;
        doc.setContent (strHtml);
        if (1 != doc.elementsByTagName("html").size()) {
            qWarning ("Unexpected number of html tags");
            return false;
        }

        QDomNodeList mainDivs = doc.elementsByTagName("html").at(0).childNodes();
        for (int i = 0; i < mainDivs.size (); i++) {
            QDomNode oneDivNode = mainDivs.at (i);
            if (!oneDivNode.isElement ()) {
                continue;
            }

            QDomElement oneDivElement = oneDivNode.toElement ();
            if (oneDivElement.tagName() != "div") {
                continue;
            }

            if (!oneDivElement.hasAttribute ("id")) {
                continue;
            }

            QString id = oneDivElement.attribute("id");
            QString strSmsRow;
            QDomNodeList subDivs = oneDivElement.elementsByTagName("div");
            for (int j = 0; j < subDivs.size (); j++) {
                if (!subDivs.at(j).isElement ()) {
                    continue;
                }

                if (!subDivs.at(j).toElement().hasAttribute("class")) {
                    continue;
                }

                bool bMessageDisplay = false;
                attrs = subDivs.at(j).toElement().attributes();
                for (int k = 0; k < attrs.size (); k++) {
                    if (attrs.item(k).toAttr().value() == "gc-message-message-display") {
                        bMessageDisplay = true;
                        break;
                    }
                }
                if (!bMessageDisplay) {
                    continue;
                }

                // Children could be either SMS rows or vmail transcription
                QDomNodeList childDivs = subDivs.at(j).toElement().childNodes();
                for (int k = 0; k < childDivs.size (); k++) {
                    if (!childDivs.at(k).isElement ()) {
                        continue;
                    }

                    // Find out if it is a div and has the sms atttribute
                    bool bInteresting = false;
                    if (childDivs.at(k).toElement().tagName () == "div") {
                        attrs = childDivs.at(k).toElement().attributes();
                        for (int l = 0; l < attrs.size (); l++) {
                            if (attrs.item(l).toAttr().value() == "gc-message-sms-row") {
                                bInteresting = true;
                                break;
                            }
                        }
                    }
                    if (bInteresting) {
                        QDomNodeList smsRow = childDivs.at(k).toElement().childNodes();
                        for (int l = 0; l < smsRow.size (); l++) {
                            if (!smsRow.at(l).isElement()) {
                                continue;
                            }

                            QDomElement smsSpan = smsRow.at(l).toElement();
                            if (smsSpan.tagName () != "span") {
                                continue;
                            }

                            attrs = smsSpan.attributes();
                            for (int m = 0; m < attrs.size (); m++) {
                                QString strTemp = smsSpan.text ().simplified ();
                                QDomAttr attr = attrs.item(m).toAttr();
                                if (attr.value() == "gc-message-sms-from") {
                                    strSmsRow += "<b>" + strTemp + "</b> ";
                                } else if (attr.value() == "gc-message-sms-text") {
                                    strSmsRow += strTemp;
                                } else if (attr.value() == "gc-message-sms-time") {
                                    strSmsRow += " <i>(" + strTemp + ")</i><br>";
                                }
                            }// loop thru the parts of a single sms
                        }//loop through sms row
                        // done with the sms. move to the next child of message display
                        continue;
                    }//if bSmsRow

                    // Its not an SMS. Check to see if it's a vmail.
                    if (childDivs.at(k).toElement().tagName () != "span") {
                        // I don't care about anything other than the div and
                        // span children of message-display
                        continue;
                    }

                    bInteresting = false;
                    QDomElement vmailSpan = childDivs.at(k).toElement();
                    attrs = vmailSpan.attributes();
                    for (int l = 0; l < attrs.size (); l++) {
                        QDomAttr attr = attrs.item(l).toAttr();
                        if (attr.name () != "class") {
                            continue;
                        }
                        if (attr.value().startsWith ("gc-word-")) {
                            bInteresting = true;
                            break;
                        }
                    }// loop thru the attributes of a single span looking for something interesting
                    if (!bInteresting) {
                        continue;
                    }

                    if (!strSmsRow.isEmpty ()) {
                        strSmsRow += ' ';
                    }
                    strSmsRow += vmailSpan.text ();
                }//loop thru children of a messages-display div
            }//loop thru sub-divs under the main divs in the document

            if (!strSmsRow.isEmpty ()) {
                mapTexts[id] = strSmsRow;
            }
        }//loop through the main divs just under the html tag
    }// if html
    return (true);
}//GVI_XMLJsonHandler::endElement

bool
GVI_XMLJsonHandler::characters (const QString &ch)
{
    strChars += ch;
    return (true);
}//GVI_XMLJsonHandler::characters

bool
GVI_XMLJsonHandler::parseJSON (const QDateTime &dtUpdate, bool &bGotOld, int &nNew)
{
    bool rv = false;
    do // Begin cleanup block (not a loop)
    {
        QString strTemp;
        QScriptEngine scriptEngine;
        strTemp = "var topObj = " + strJson;
        scriptEngine.evaluate (strTemp);

        strTemp = "var msgParams = []; "
                  "var msgList = []; "
                  "for (var msgId in topObj[\"messages\"]) { "
                  "    msgList.push(msgId); "
                  "}";
        scriptEngine.evaluate (strTemp);
        if (scriptEngine.hasUncaughtException ()) {
            strTemp = QString ("Uncaught exception executing script : %1")
                      .arg (scriptEngine.uncaughtException ().toString ());
            qWarning () << strTemp;
            break;
        }

        qint32 nMsgCount = scriptEngine.evaluate("msgList.length;").toInt32 ();
        if (bEmitLog) qDebug() << QString("message count = %1").arg (nMsgCount);

        qint32 nOldMsgs = 0;

        for (qint32 i = 0; i < nMsgCount; i++) {
            strTemp = QString(
                    "msgParams = []; "
                    "for (var params in topObj[\"messages\"][msgList[%1]]) { "
                    "    msgParams.push(params); "
                    "}").arg(i);
            scriptEngine.evaluate (strTemp);
            if (scriptEngine.hasUncaughtException ()) {
                strTemp = QString ("Uncaught exception in message loop: %1")
                          .arg (scriptEngine.uncaughtException ().toString ());
                qWarning () << strTemp;
                break;
            }

            qint32 nParams =
            scriptEngine.evaluate ("msgParams.length;").toInt32 ();

            GVInboxEntry inboxEntry;
            for (qint32 j = 0; j < nParams; j++) {
                strTemp = QString("msgParams[%1];").arg (j);
                QString strPName = scriptEngine.evaluate (strTemp).toString ();
                strTemp = QString(
                          "topObj[\"messages\"][msgList[%1]][msgParams[%2]];")
                            .arg (i)
                            .arg (j);
                QString strVal = scriptEngine.evaluate (strTemp).toString ();

                if (strPName == "id") {
                    inboxEntry.id = strVal;
                } else if (strPName == "phoneNumber") {
                    inboxEntry.strPhoneNumber = strVal;
                } else if (strPName == "displayNumber") {
                    inboxEntry.strDisplayNumber = strVal;
                } else if (strPName == "startTime") {
                    bool bOk = false;
                    quint64 iVal = strVal.toULongLong (&bOk) / 1000;
                    if (bOk) {
                        inboxEntry.startTime = QDateTime::fromTime_t (iVal);
                    }
                } else if (strPName == "isRead") {
                    inboxEntry.bRead = (strVal == "true");
                } else if (strPName == "isSpam") {
                    inboxEntry.bSpam = (strVal == "true");
                } else if (strPName == "isTrash") {
                    inboxEntry.bTrash = (strVal == "true");
                } else if (strPName == "star") {
                    inboxEntry.bStar = (strVal == "true");
                } else if (strPName == "labels") {
                    if (strVal.contains ("placed")) {
                        inboxEntry.Type = GVIE_Placed;
                    } else if (strVal.contains ("received")) {
                        inboxEntry.Type = GVIE_Received;
                    } else if (strVal.contains ("missed")) {
                        inboxEntry.Type = GVIE_Missed;
                    } else if (strVal.contains ("voicemail")) {
                        inboxEntry.Type = GVIE_Voicemail;
                    } else if (strVal.contains ("sms")) {
                        inboxEntry.Type = GVIE_TextMessage;
                    } else {
                        if (bEmitLog) qWarning () << "Unknown label" << strVal;
                    }
                } else if (strPName == "displayStartDateTime") {
                } else if (strPName == "displayStartTime") {
                } else if (strPName == "relativeStartTime") {
                } else if (strPName == "note") {
                    inboxEntry.strNote = strVal;
                } else if (strPName == "type") {
                } else if (strPName == "children") {
                } else {
                    if (bEmitLog)
                        qDebug () << QString ("param = %1. value = %2")
                                        .arg (strPName) .arg (strVal);
                }
            }

            if (0 == inboxEntry.id.size()) {
                qWarning ("Invalid ID");
                continue;
            }
            if (0 == inboxEntry.strPhoneNumber.size()) {
                qWarning ("Invalid Phone number");
                inboxEntry.strPhoneNumber = "Unknown";
            }
            if (0 == inboxEntry.strDisplayNumber.size()) {
                inboxEntry.strDisplayNumber = "Unknown";
            }
            if (!inboxEntry.startTime.isValid ()) {
                qWarning ("Invalid start time");
                continue;
            }

            // Check to see if it is too old to show
            if (dtUpdate.isValid () && (dtUpdate >= inboxEntry.startTime))
            {
                nOldMsgs++;
                if (1 == nOldMsgs) {
                    if (bEmitLog) qDebug ("Started getting old entries.");
                    bGotOld = true;
                } else {
                    if (bEmitLog) qDebug ("Another old entry");
                }
            }

            // Pick up the text from the parsed HTML
            if (((GVIE_TextMessage == inboxEntry.Type) ||
                 (GVIE_Voicemail == inboxEntry.Type)) &&
                (mapTexts.contains (inboxEntry.id)))
            {
                inboxEntry.strText = mapTexts[inboxEntry.id];
            }

            // emit the inbox element
            emit oneElement (inboxEntry);
            nUsableMsgs++;
        }

        nNew = nUsableMsgs - nOldMsgs;
        if (bEmitLog)
            qDebug () << QString ("Usable %1, old %2, new %3")
                            .arg (nUsableMsgs).arg (nOldMsgs).arg (nNew);

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//GVI_XMLJsonHandler::parseJSON

qint32
GVI_XMLJsonHandler::getUsableMsgsCount ()
{
    return (nUsableMsgs);
}//GVI_XMLJsonHandler::getUsableMsgsCount

void
GVI_XMLJsonHandler::setEmitLog (bool enable)
{
    bEmitLog = enable;
}//GVI_XMLJsonHandler::setEmitLog
