/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

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

bool
MobileWebPage::acceptNavigationRequest (QWebFrame *frame,
                                        const QNetworkRequest & /*request*/,
                                        QWebPage::NavigationType /*type*/)
{
    qWarning() << "acceptNavigationRequest frame = %1" << (void *)frame;
    return true;
}//MobileWebPage::acceptNavigationRequest

QWebPage *
MobileWebPage::createWindow (QWebPage::WebWindowType type)
{
    qWarning("Webpage requested a new page");
    return NULL;
}//MobileWebPage::createWindow
