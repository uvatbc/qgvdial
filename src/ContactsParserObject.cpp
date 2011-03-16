#include "ContactsParserObject.h"

#include "Singletons.h"
#include "ContactsXmlHandler.h"
#include "ContactsModel.h"

ContactsParserObject::ContactsParserObject (QByteArray data, QObject *parent)
: QObject(parent)
, byData (data)
{
}//ContactsParserObject::ContactsParserObject

void
ContactsParserObject::doWork ()
{
    QXmlInputSource inputSource;
    QXmlSimpleReader simpleReader;
    ContactsXmlHandler contactsHandler;

    inputSource.setData (byData);

    QObject::connect (&contactsHandler, SIGNAL (status(const QString &, int)),
                       this,            SIGNAL (status(const QString &, int)));

    QObject::connect (
        &contactsHandler, SIGNAL (oneContact (const ContactInfo &)),
         this,            SIGNAL (gotOneContact (const ContactInfo &)));

    simpleReader.setContentHandler (&contactsHandler);
    simpleReader.setErrorHandler (&contactsHandler);

    bool rv = simpleReader.parse (&inputSource, false);

    QString msg = QString("Total contacts: %1. Usable: %2")
            .arg (contactsHandler.getTotalContacts ())
            .arg (contactsHandler.getUsableContacts ());
    emit status(msg);

    emit done(rv);
}//ContactsParserObject::doWork
