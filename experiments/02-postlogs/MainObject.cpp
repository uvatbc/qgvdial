#include "MainObject.h"

#define LOC_ROOT "http://ec2-50-18-18-251.us-west-1.compute.amazonaws.com"
#define CONTENT_IS_FORM "application/x-www-form-urlencoded"
#define CONTENT_IS_TEXT "text/plain"

MainObject::MainObject(QObject *parent)
: QObject(parent)
{
    connect(&mgr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onFinished(QNetworkReply*)));

//    QTimer::singleShot(10, this, SLOT(doPost()));
    QTimer::singleShot(10, this, SLOT(doGet()));
}//MainObject::MainObject

void
MainObject::doPost()
{
    QUrl url(LOC_ROOT "/qgvdial/postLog.py");
    url.addQueryItem("param1", "woh");
    url.addQueryItem("param2", "wah");

    QNetworkRequest req(url);
    req.setHeader (QNetworkRequest::ContentTypeHeader, CONTENT_IS_FORM);
    reply = mgr.post(req, QByteArray("This is the post data"));
}//MainObject::doPost

void
MainObject::doGet()
{
    QUrl url(LOC_ROOT "/qgvdial/getLogLocation.py");

    QNetworkRequest req(url);
    reply = mgr.get(req);
}//MainObject::doGet

void
MainObject::onFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error:" << reply->error();
    }

    QString tmp = reply->readAll();
    qDebug() << tmp;

    qApp->quit();
}//MainObject::onFinished

