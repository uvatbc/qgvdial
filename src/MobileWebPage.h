/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

#ifndef __MOBILEWEBPAGE_H__
#define __MOBILEWEBPAGE_H__

#include "global.h"
#include <QWebPage>

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class MobileWebPage : public QWebPage
{
    Q_OBJECT

public:
    MobileWebPage (QObject *parent = NULL);

public slots:
    void setUA (bool bSetIphone = false);

protected:
    QString userAgentForUrl (const QUrl & url) const;

    inline bool acceptNavigationRequest (QWebFrame * /*frame*/,
                                         const QNetworkRequest & /*request*/,
                                         QWebPage::NavigationType /*type*/)
    { return true; }

    QWebPage *createWindow (QWebPage::WebWindowType type);

protected:
    bool bUAIsIphone;
};

#endif //__MOBILEWEBPAGE_H__
