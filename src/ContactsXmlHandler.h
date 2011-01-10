#ifndef __CONTACTSXMLHANDLER_H__
#define __CONTACTSXMLHANDLER_H__

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class ContactsXmlHandler : public QObject, public QXmlDefaultHandler
{
    Q_OBJECT

public:
    ContactsXmlHandler (QObject *parent = 0);
    virtual ~ContactsXmlHandler (void);

    quint32 getTotalContacts ();
    quint32 getUsableContacts ();

signals:
    void status (const QString &text, int timeout = 2000);

    //! Emitted for every contact parsed from the XML
    void oneContact (const ContactInfo &contactInfo);

protected:
    bool startElement (const QString        &namespaceURI,
                       const QString        &localName   ,
                       const QString        &qName       ,
                       const QXmlAttributes &atts        );

    bool endElement (const QString &namespaceURI,
                     const QString &localName   ,
                     const QString &qName       );

    bool characters (const QString &ch);

protected:
    bool        bEntryStarted;

    quint32     countContacts;
    quint32     countUsableContacts;

    QString     strCurrentChars;

    PhoneInfo   currPhone;
    ContactInfo currInfo;
};

#endif //__CONTACTSXMLHANDLER_H__
