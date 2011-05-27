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

#include "CaptchaWidget.h"

CaptchaWidget::CaptchaWidget (const QString        &strLink,
                                    QWidget        *parent ,
                                    Qt::WindowFlags f)
: QWidget (parent, f)
, grid (this)
, webCaptcha (this)
, edCaptcha (this)
{
    grid.addWidget (&webCaptcha , 0,0);
    grid.addWidget (&edCaptcha  , 1,0);
    this->setLayout (&grid);

    QObject::connect (&webCaptcha, SIGNAL (loadFinished (bool)),
                       this      , SLOT   (captchaLoaded (bool)));
    webCaptcha.load (QUrl (strLink));

    this->hide ();
}//CaptchaWidget::CaptchaWidget

CaptchaWidget::~CaptchaWidget()
{
}//CaptchaWidget::~CaptchaWidget

void
CaptchaWidget::captchaLoaded (bool bOk)
{
    QObject::disconnect (&webCaptcha, SIGNAL (loadFinished (bool)),
                          this      , SLOT   (captchaLoaded (bool)));

    do { // Begin cleanup block (not a loop)
    	if (!bOk)
    	{
            emit done(false, QString());
            this->deleteLater ();
            break;
    	}

        QObject::connect (
            &edCaptcha, SIGNAL (returnPressed ()),
             this     , SLOT   (onEdEnter ()));
        this->show ();
    } while (0); // End cleanup block (not a loop)
}//CaptchaWidget::captchaLoaded

void
CaptchaWidget::onEdEnter ()
{
    emit done(true, edCaptcha.text ());
    this->deleteLater ();
}//CaptchaWidget::onEdEnter
