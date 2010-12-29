#ifndef GVH_XMLJSONHANDLER_H
#define GVH_XMLJSONHANDLER_H

#include "global.h"
#include <QtXml>

class GVH_XMLJsonHandler : public QObject, public QXmlDefaultHandler
{
    Q_OBJECT

public:
    explicit GVH_XMLJsonHandler(QObject *parent = 0);
    bool parseJSON (const QDateTime &dtUpdate, bool &bGotOld);
    qint32 getUsableMsgsCount ();

signals:
    void oneElement (const GVHistoryEvent &element);

protected:
    bool startElement (const QString        &namespaceURI,
                       const QString        &localName   ,
                       const QString        &qName       ,
                       const QXmlAttributes &atts        );

    bool endElement (const QString &namespaceURI,
                     const QString &localName   ,
                     const QString &qName       );

    bool characters (const QString &ch);

private:
    QString strChars;
    QString strJson;
    QString strHtml;
    qint32 nUsableMsgs;
};

#endif // GVH_XMLJSONHANDLER_H
