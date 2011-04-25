#include "GVI_SMS_Handler.h"

GVI_SMS_Handler::GVI_SMS_Handler (QObject *parent)
: QObject(parent)
, uDepth (0)
, bTextStarted (false)
, bTextFragmentStarted (false)
, bEmitLog (true)
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
            uDepthSinceTextStart++;

            if (atts.value ("class").contains ("gc-message-message-display")) {
                if (bEmitLog) qDebug ("Found a text or voicemail");
                strChars.clear ();
                bTextStarted = true;
                uDepthSinceTextStart = 0;
            }

            if (uDepth != 1) {
                if ((!atts.value ("id").isEmpty ()) &&
                    (atts.value ("class").contains ("gc-message"))) {
                    if (bEmitLog) qDebug ("Got next ID!!!");
                }
                break;
            }

            id = atts.value ("id");
        } else if (localName == "span") {
            if (bTextStarted) {
                if ((atts.value ("class") == "gc-word-high") ||
                    (atts.value ("class") == "gc-word-med1") ||
                    (atts.value ("class") == "gc-word-med2") ||
                    (atts.value ("class") == "gc-message-sms-text"))
                {
                    strChars.clear ();
                    bTextFragmentStarted = true;
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
        if (bTextStarted && bTextFragmentStarted) {
            strVmail += strChars + " ";
            bTextFragmentStarted = false;
        }
    } else if (localName == "div") {
        uDepth--;
        uDepthSinceTextStart--;

        if (bTextStarted) {
            if (strVmail.endsWith (' ')) {
                strVmail.truncate (strVmail.size()-1);
            }
            mapTexts[id] = strVmail;
            strVmail.clear ();
            if (uDepthSinceTextStart <= 0) {
                bTextStarted = false;
            }
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

void
GVI_SMS_Handler::setEmitLog (bool enable)
{
    bEmitLog = enable;
}//GVI_SMS_Handler::setEmitLog
