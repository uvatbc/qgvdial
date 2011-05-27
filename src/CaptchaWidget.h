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

#ifndef __CAPTCHAWIDGET_H__
#define __CAPTCHAWIDGET_H__

#include "global.h"
#include <QtWebKit>

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class CaptchaWidget : public QWidget
{
    Q_OBJECT

public:
    CaptchaWidget (const QString        &strLink   ,
                         QWidget        *parent = 0,
                         Qt::WindowFlags f      = 0);
    virtual ~CaptchaWidget();

signals:
    void done (bool bOk, const QString &strCaptcha);

private slots:
    void captchaLoaded (bool bOk);
    void onEdEnter ();

private:
    QGridLayout grid;
    QWebView    webCaptcha;
    QLineEdit   edCaptcha;
};

#endif //__CAPTCHAWIDGET_H__
