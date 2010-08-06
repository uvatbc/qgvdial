#include "QGVDbusServer.h"

QGVDbusServerHelper::QGVDbusServerHelper (QObject *parent)
: QObject (parent)
{
}//QGVDbusServerHelper::QGVDbusServerHelper

void
QGVDbusServerHelper::emitDialNow (const QString &strNumber)
{
    emit dialNow (strNumber);
}//QGVDbusServerHelper::emitDialNow

QGVDbusServer::QGVDbusServer (QObject *parent)
: QDBusAbstractAdaptor(parent)
, helper (this)
{
}//QGVDbusServer::QGVDbusServer

void
QGVDbusServer::Call (const QString &strNumber)
{
    // Make a call
    helper.emitDialNow (strNumber);
}//QGVDbusServer::Call

void
QGVDbusServer::addCallReceiver (QObject *receiver, const char *method)
{
    QObject::connect (&helper, SIGNAL (dialNow (const QString &)),
                      receiver, method);
}//QGVDbusServer::addCallReceiver

void
QGVDbusServer::delCallReceiver (QObject *receiver, const char *method)
{
    QObject::disconnect (&helper, SIGNAL (dialNow (const QString &)),
                          receiver, method);
}//QGVDbusServer::delCallReceiver
