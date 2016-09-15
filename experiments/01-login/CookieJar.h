#ifndef COOKIEJAR_H
#define COOKIEJAR_H

#include "global.h"
#include <QObject>

class CookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    explicit CookieJar(QObject *parent = 0);

    QList<QNetworkCookie> getAllCookies() const;
    void setNewCookies(const QList<QNetworkCookie> &cookies);
};

#endif // COOKIEJAR_H
