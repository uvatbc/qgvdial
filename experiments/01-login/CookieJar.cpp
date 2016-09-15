#include "CookieJar.h"

CookieJar::CookieJar(QObject *parent)
: QNetworkCookieJar(parent)
{
}//CookieJar::CookieJar

void
CookieJar::setNewCookies(const QList<QNetworkCookie> &cookies)
{
    setAllCookies (cookies);
}//CookieJar::setNewCookies

QList<QNetworkCookie>
CookieJar::getAllCookies() const
{
    return allCookies ();
}//CookieJar::getAllCookies
