#include "GVH_XMLJsonHandler.h"
#include <QtScript>

GVH_XMLJsonHandler::GVH_XMLJsonHandler (QObject *parent)
: QObject (parent)
, nUsableMsgs (0)
{
}//GVH_XMLJsonHandler::GVH_XMLJsonHandler

bool
GVH_XMLJsonHandler::startElement (const QString        & /*namespaceURI*/,
                                  const QString        & /*localName   */,
                                  const QString        & /*qName       */,
                                  const QXmlAttributes & /*atts        */)
{
    //emit log (QString ("start name=%1").arg (localName));
    strChars.clear ();
    return (true);
}//GVH_XMLJsonHandler::startElement

bool
GVH_XMLJsonHandler::endElement (const QString & /*namespaceURI*/,
                               const QString &localName   ,
                               const QString & /*qName       */)
{
    if (localName == "json") {
        strJson += strChars;
        emit log ("Got json characters");
    }

    //emit log (QString ("stop name=%1").arg (localName));
    return (true);
}//GVH_XMLJsonHandler::endElement

bool
GVH_XMLJsonHandler::characters (const QString &ch)
{
    strChars += ch;
    return (true);
}//GVH_XMLJsonHandler::characters

bool
GVH_XMLJsonHandler::parseJSON (const QDateTime &dtUpdate, bool &bGotOld)
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
            emit log (strTemp);
            break;
        }

        qint32 nMsgCount = scriptEngine.evaluate("msgList.length;").toInt32 ();
        emit log (QString ("message count = %1").arg (nMsgCount));

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
                emit log (strTemp);
                break;
            }

            qint32 nParams =
            scriptEngine.evaluate ("msgParams.length;").toInt32 ();

//            emit log (QString ("Msg %1 has %2 params").arg (i).arg (nParams));

            GVHistoryEvent oneHistory;
            for (qint32 j = 0; j < nParams; j++) {
                strTemp = QString("msgParams[%1];").arg (j);
                QString strPName = scriptEngine.evaluate (strTemp).toString ();
                strTemp = QString(
                          "topObj[\"messages\"][msgList[%1]][msgParams[%2]];")
                            .arg (i)
                            .arg (j);
                QString strVal = scriptEngine.evaluate (strTemp).toString ();

                if (strPName == "id") {
                    oneHistory.id = strVal;
                } else if (strPName == "phoneNumber") {
                    oneHistory.strPhoneNumber = strVal;
                } else if (strPName == "displayNumber") {
                    oneHistory.strDisplayNumber = strVal;
                } else if (strPName == "startTime") {
                    bool bOk = false;
                    quint64 iVal = strVal.toULongLong (&bOk) / 1000;
                    if (bOk) {
                        oneHistory.startTime = QDateTime::fromTime_t (iVal);
                    }
                } else if (strPName == "isRead") {
                    oneHistory.bRead = (strVal == "true");
                } else if (strPName == "isSpam") {
                    oneHistory.bSpam = (strVal == "true");
                } else if (strPName == "isTrash") {
                    oneHistory.bTrash = (strVal == "true");
                } else if (strPName == "star") {
                    oneHistory.bStar = (strVal == "true");
                } else if (strPName == "labels") {
                    if (strVal.contains ("placed")) {
                        oneHistory.Type = GVHE_Placed;
                    } else if (strVal.contains ("received")) {
                        oneHistory.Type = GVHE_Received;
                    } else if (strVal.contains ("missed")) {
                        oneHistory.Type = GVHE_Missed;
                    } else if (strVal.contains ("voicemail")) {
                        oneHistory.Type = GVHE_Voicemail;
                    } else if (strVal.contains ("sms")) {
                        oneHistory.Type = GVHE_TextMessage;
                    } else {
                        emit log (QString ("Unknown labels %1").arg (strVal));
                    }
                } else if (strPName == "displayStartDateTime") {
                } else if (strPName == "displayStartTime") {
                } else if (strPName == "relativeStartTime") {
                } else if (strPName == "note") {
                } else if (strPName == "type") {
                } else if (strPName == "children") {
                } else {
                    emit log (QString ("param = %1. value = %2")
                                .arg (strPName)
                                .arg (strVal));
                }
            }

            if (0 == oneHistory.id.size()) {
                emit log ("Invalid ID", 5);
                continue;
            }
            if (0 == oneHistory.strPhoneNumber.size()) {
                emit log ("Invalid Phone number", 5);
                continue;
            }
            if (0 == oneHistory.strDisplayNumber.size()) {
                oneHistory.strDisplayNumber = "Unknown";
            }
            if (!oneHistory.startTime.isValid ()) {
                emit log ("Invalid start time", 5);
                continue;
            }

            // Check to see if it is too old to show
            if (dtUpdate.isValid () && (dtUpdate >= oneHistory.startTime))
            {
                nOldMsgs++;
                if (1 == nOldMsgs) {
                    emit log ("Started getting old entries.");
                    bGotOld = true;
                } else {
                    emit log ("Another old entry");
                }
            }

            // emit the history element
            emit oneElement (oneHistory);
            nUsableMsgs++;
        }

        emit log (QString ("Valid messages = %1").arg (nUsableMsgs));
        emit log (QString ("Old messages = %1").arg (nOldMsgs));

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//GVH_XMLJsonHandler::parseJSON

qint32
GVH_XMLJsonHandler::getUsableMsgsCount ()
{
    return (nUsableMsgs);
}//GVH_XMLJsonHandler::getUsableMsgsCount
