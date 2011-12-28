#include "MyXmlErrorHandler.h"

MyXmlErrorHandler::MyXmlErrorHandler(QObject *parent)
: QAbstractMessageHandler(parent)
{
}//MyXmlErrorHandler::MyXmlErrorHandler

void
MyXmlErrorHandler::handleMessage (QtMsgType type, const QString &description,
                                  const QUrl & /*identifier*/,
                                  const QSourceLocation &sourceLocation)
{
    QString msg = QString("XML message: %1, at uri= %2 "
                          "line %3 column %4")
                .arg(description)
                .arg(sourceLocation.uri ().toString ())
                .arg(sourceLocation.line ())
                .arg(sourceLocation.column ());

    switch (type)
    {
    case QtDebugMsg:
        qDebug() << msg;
        break;
    case QtWarningMsg:
        qWarning() << msg;
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        qCritical() << msg;
        break;
    }
}//MyXmlErrorHandler::handleMessage
