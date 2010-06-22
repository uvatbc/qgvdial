#ifndef __MOBILEWEBPAGE_H__
#define __MOBILEWEBPAGE_H__

#include <QWebPage>

class MobileWebPage : public QWebPage
{
public:
    MobileWebPage (QObject *parent = NULL);

protected:
    QString userAgentForUrl (const QUrl & url) const;
};

#endif //__MOBILEWEBPAGE_H__
