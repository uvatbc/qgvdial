#ifndef __MYXMLERRORHANDLER__
#define __MYXMLERRORHANDLER__

#include "global.h"
#include <QObject>
#include <QtXmlPatterns>

class MyXmlErrorHandler : public QAbstractMessageHandler
{
public:
    MyXmlErrorHandler(QObject *parent = NULL);

protected:
    void handleMessage (QtMsgType type, const QString &description,
                        const QUrl &identifier,
                        const QSourceLocation &sourceLocation);
};

#endif//__MYXMLERRORHANDLER__

