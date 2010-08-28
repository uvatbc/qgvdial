#ifndef __MOBILEWEBPAGE_H__
#define __MOBILEWEBPAGE_H__

#include <QWebPage>

class MobileWebPage : public QWebPage
{
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
