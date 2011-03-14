#include "SymbianCallInitiator.h"

SymbianCallInitiator::SymbianCallInitiator (QObject *parent)
: CalloutInitiator(parent)
{
}//SymbianCallInitiator::SymbianCallInitiator

QString
SymbianCallInitiator::name ()
{
    return "Phone";
}//SymbianCallInitiator::name

QString
SymbianCallInitiator::selfNumber ()
{
    return "unknown";
}//SymbianCallInitiator::selfNumber

void
SymbianCallInitiator::initiateCall (const QString &strDestination)
{
    qDebug ("SymbianCallInitiator::initiateCall");
}//SymbianCallInitiator::initiateCall
