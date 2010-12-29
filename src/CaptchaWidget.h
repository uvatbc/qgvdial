#ifndef __CAPTCHAWIDGET_H__
#define __CAPTCHAWIDGET_H__

#include "global.h"
#include <QtWebKit>

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
