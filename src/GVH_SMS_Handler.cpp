#include "GVH_SMS_Handler.h"

GVH_SMS_Handler::GVH_SMS_Handler (QObject *parent)
: QObject(parent)
, uDepth (0)
, bTextStarted (false)
{
}//GVH_SMS_Handler::GVH_SMS_Handler

bool
GVH_SMS_Handler::startElement (const QString        & /*namespaceURI*/,
                               const QString        &localName   ,
                               const QString        & /*qName       */,
                               const QXmlAttributes &atts        )
{
    do {
        if (localName == "div") {
            uDepth++;
            if (uDepth != 1) {
                if ((!atts.value ("id").isEmpty ()) &&
                    (atts.value ("class").contains ("gc-message"))) {
                    qDebug ("Got next ID!!!");
                }
                break;
            }

            id = atts.value ("id");
        } else if (localName == "span") {
            if (atts.value ("class") != "gc-message-sms-text") {
                break;
            }

            bTextStarted = true;
            strChars.clear ();
        }
    } while (0);

    return (true);
}//GVH_SMS_Handler::startElement

bool
GVH_SMS_Handler::endElement (const QString & /*namespaceURI*/,
                             const QString &localName   ,
                             const QString & /*qName       */)
{
    if (bTextStarted && localName == "span") {
        bTextStarted = false;

        mapTexts[id] = strChars;
    } else if (localName == "div") {
        uDepth--;
    }
    return (true);
}//GVH_SMS_Handler::endElement

bool
GVH_SMS_Handler::characters (const QString &ch)
{
    strChars += ch;
    return (true);
}//GVH_SMS_Handler::characters
