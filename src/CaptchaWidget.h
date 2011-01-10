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
