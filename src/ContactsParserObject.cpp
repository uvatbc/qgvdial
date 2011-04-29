#include "ContactsParserObject.h"
#include "ContactsXmlHandler.h"

ContactsParserObject::ContactsParserObject (QByteArray data, QObject *parent)
: QObject(parent)
, byData (data)
, bEmitLog (true)
{
}//ContactsParserObject::ContactsParserObject

void
ContactsParserObject::doWork ()
{
    QXmlInputSource inputSource;
    QXmlSimpleReader simpleReader;
    ContactsXmlHandler contactsHandler;
    contactsHandler.setEmitLog (bEmitLog);

    inputSource.setData (byData);

    QObject::connect (&contactsHandler, SIGNAL (status(const QString &, int)),
                       this,            SIGNAL (status(const QString &, int)));

    QObject::connect (
        &contactsHandler, SIGNAL (oneContact (const ContactInfo &)),
         this,            SIGNAL (gotOneContact (const ContactInfo &)));

    simpleReader.setContentHandler (&contactsHandler);
    simpleReader.setErrorHandler (&contactsHandler);

    bool rv = simpleReader.parse (&inputSource, false);

    if (!rv) {
        qDebug() << "Contacts parser failed to parse. Data =" << byData;
    }

    QString msg = QString("Total contacts: %1. Usable: %2")
            .arg (contactsHandler.getTotalContacts ())
            .arg (contactsHandler.getUsableContacts ());
    if (bEmitLog || (contactsHandler.getUsableContacts () != 0)) {
        emit status(msg);
    }

    emit done(rv);
}//ContactsParserObject::doWork

void
ContactsParserObject::setEmitLog (bool enable /*= true*/)
{
    bEmitLog = enable;
}//ContactsParserObject::setEmitLog
