#include "global.h"
#include "MobileWebPage.h"

MobileWebPage::MobileWebPage (QObject *parent/* = NULL*/)
: QWebPage(parent)
, bUAIsIphone (false)
{
}//MobileWebPage::MobileWebPage

QString
MobileWebPage::userAgentForUrl (const QUrl &) const
{
    if (bUAIsIphone)
        return (UA_IPHONE);
    else
        return (UA_N900);
}//MobileWebPage::userAgentForUrl

void
MobileWebPage::setUA (bool bSetIphone /*= false*/)
{
    bUAIsIphone = bSetIphone;
}//MobileWebPage::setUA
