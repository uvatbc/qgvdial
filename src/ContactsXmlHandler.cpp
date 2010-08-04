#include "ContactsXmlHandler.h"

ContactsXmlHandler::ContactsXmlHandler (QObject *parent)
: QObject(parent)
, QXmlDefaultHandler ()
, bEntryStarted (false)
, countContacts (0)
, countUsableContacts (0)
{
}//ContactsXmlHandler::ContactsXmlHandler

ContactsXmlHandler::~ContactsXmlHandler(void)
{
}//ContactsXmlHandler::~ContactsXmlHandler

bool
ContactsXmlHandler::startElement (const QString        &namespaceURI,
                                  const QString        &localName   ,
                                  const QString        &qName       ,
                                  const QXmlAttributes &atts        )
{
    QString msg;

    do // Begin cleanup block (not a loop)
    {
        strCurrentChars.clear ();

        if (localName == "entry")
        {
            bEntryStarted = true;
            countContacts++;

            currInfo.init ();
            break;
        }

        if (!bEntryStarted)
        {
            break;
        }

        if (localName == "phoneNumber")
        {
            currPhone.init ();
            QString rel = atts.value ("rel");
            if (rel.endsWith ("mobile"))
            {
                currPhone.Type = PType_Mobile;
            }
            else if (rel.endsWith ("home"))
            {
                currPhone.Type = PType_Home;
            }
            else
            {
                currPhone.Type = PType_Other;
            }
        }
    } while (0); // End cleanup block (not a loop)
    return (true);
}//ContactsXmlHandler::startElement

bool
ContactsXmlHandler::endElement (const QString &namespaceURI,
                                const QString &localName   ,
                                const QString &qName       )
{
    QString msg;
    do // Begin cleanup block (not a loop)
    {
        if (localName == "id")
        {
            currInfo.strId = strCurrentChars;
        }
        if (localName == "title")
        {
            currInfo.strTitle = strCurrentChars;
        }
        if (localName == "phoneNumber")
        {
            currPhone.strNumber = strCurrentChars;
            currInfo.arrPhones += currPhone;
        }

        if (localName == "entry")
        {
            bEntryStarted = false;

            if (0 != currInfo.arrPhones.size ())
            {
                countUsableContacts ++;
                emit oneContact (currInfo);
            }
        }
    } while (0); // End cleanup block (not a loop)
    return (true);
}//ContactsXmlHandler::endElement

bool
ContactsXmlHandler::characters (const QString &ch)
{
    strCurrentChars += ch;

    return (true);
}//ContactsXmlHandler::characters

quint32
ContactsXmlHandler::getTotalContacts ()
{
    return (countContacts);
}//ContactsXmlHandler::getTotalContacts

quint32
ContactsXmlHandler::getUsableContacts ()
{
    return (countUsableContacts);
}//ContactsXmlHandler::getUsableContacts
