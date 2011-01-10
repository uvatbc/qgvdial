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

protected:
    bool bUAIsIphone;
};

#endif //__MOBILEWEBPAGE_H__
