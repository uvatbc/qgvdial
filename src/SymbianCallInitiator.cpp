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
