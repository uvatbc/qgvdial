#include "global.h"
#include "MobileWebPage.h"

MobileWebPage::MobileWebPage (QObject *parent/* = NULL*/):
QWebPage(parent)
{
}//MobileWebPage::MobileWebPage

QString
MobileWebPage::userAgentForUrl (const QUrl & url) const
{
    return ("Mozilla/5.0 (X11; U; Linux armv7l; en-GB; rv:1.9.2a1pre) Gecko/20090928 Firefox/3.5 Maemo Browser 1.4.1.21 RX-51 N900");
    Q_UNUSED (url);
}//MobileWebPage::userAgentForUrl
