#ifndef COOKIEJAR_H
#define COOKIEJAR_H

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
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
