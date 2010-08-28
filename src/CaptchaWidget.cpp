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
