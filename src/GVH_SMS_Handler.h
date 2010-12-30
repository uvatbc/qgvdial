#ifndef GVH_SMS_HANDLER_H
#define GVH_SMS_HANDLER_H

#include "global.h"
#include <QtXml>

class GVH_SMS_Handler : public QObject, public QXmlDefaultHandler
{
    Q_OBJECT
public:
    explicit GVH_SMS_Handler(QObject *parent = 0);

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
    QString id;

    quint32 uDepth;
    bool bTextStarted;

public:
    QMap<QString, QString> mapTexts;
};

#endif // GVH_SMS_HANDLER_H
