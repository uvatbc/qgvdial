#ifndef COOKIEJAR_H
#define COOKIEJAR_H

#include "global.h"

class CookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    explicit CookieJar(QObject *parent = 0);

    QList<QNetworkCookie> getAllCookies() const;
    void setNewCookies(const QList<QNetworkCookie> &cookies);

signals:

public slots:

};

#endif // COOKIEJAR_H
