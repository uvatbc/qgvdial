#ifndef CONTACTSPARSEROBJECT_H
#define CONTACTSPARSEROBJECT_H

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class ContactsParserObject : public QObject
{
    Q_OBJECT

public:
    ContactsParserObject(QByteArray data, QObject *parent = 0);
    void setEmitLog (bool enable = true);

signals:
    //! Status emitter for status bar
    void status(const QString &strText, int timeout = 2000);
    // Emitted when one contact is parsed out of the XML
    void gotOneContact (const ContactInfo &contactInfo);
    // Emitted when work is done
    void done(bool rv);

public slots:
    void doWork ();

private:
    QByteArray  byData;

    bool        bEmitLog;
};

#endif // CONTACTSPARSEROBJECT_H
