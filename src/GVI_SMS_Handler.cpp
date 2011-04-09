#include "GVI_SMS_Handler.h"

GVI_SMS_Handler::GVI_SMS_Handler (QObject *parent)
: QObject(parent)
, uDepth (0)
, bTextStarted (false)
, bVmailStarted (false)
, bVmailfragmentStarted (false)
{
}//GVI_SMS_Handler::GVI_SMS_Handler

bool
GVI_SMS_Handler::startElement (const QString        & /*namespaceURI*/,
                               const QString        &localName   ,
                               const QString        & /*qName       */,
                               const QXmlAttributes &atts        )
{
    do {
        if (localName == "div") {
            uDepth++;

            if (atts.value ("class").contains ("gc-message-message-display")) {
                qDebug ("Found a voicemail");
                strChars.clear ();
                bVmailStarted = true;
            }

            if (uDepth != 1) {
                if ((!atts.value ("id").isEmpty ()) &&
                    (atts.value ("class").contains ("gc-message"))) {
                    qDebug ("Got next ID!!!");
                }
                break;
            }

            id = atts.value ("id");
        } else if (localName == "span") {
            if (atts.value ("class") == "gc-message-sms-text") {
                bTextStarted = true;
                strChars.clear ();
                break;
            }

            if (bVmailStarted) {
                if ((atts.value ("class") == "gc-word-high") ||
                    (atts.value ("class") == "gc-word-med1") ||
                    (atts.value ("class") == "gc-word-med2"))
                {
                    strChars.clear ();
                    bVmailfragmentStarted = true;
                }
            }
        }
    } while (0);

    return (true);
}//GVI_SMS_Handler::startElement

bool
GVI_SMS_Handler::endElement (const QString & /*namespaceURI*/,
                             const QString &localName   ,
                             const QString & /*qName       */)
{
    if (localName == "span") {
        if (bTextStarted) {
            mapTexts[id] = strChars;
            bTextStarted = false;
        } else if (bVmailfragmentStarted) {
            strVmail += strChars + " ";
        }
    } else if (localName == "div") {
        uDepth--;

        if (bVmailStarted) {
            mapTexts[id] = strVmail;
            bVmailStarted = false;
        }
    }
    return (true);
}//GVI_SMS_Handler::endElement

bool
GVI_SMS_Handler::characters (const QString &ch)
{
    strChars += ch;
    return (true);
}//GVI_SMS_Handler::characters
