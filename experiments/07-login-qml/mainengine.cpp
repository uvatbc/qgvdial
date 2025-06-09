#include "mainengine.h"
#include <QQmlProperty>

#define URL_GV_MAINPAGE "https://voice.google.com"
#define UA_ANDROID_NORD_N200 "Mozilla/5.0 (Linux; Android 12; DE2118) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/112.0.0.0 Mobile Safari/537.36"
#define UA_IPHONE_16E        "Mozilla/5.0 (iPhone17,5; CPU iPhone OS 18_3_2 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Mobile/15E148 FireKeepers/1.7.0"

MainEngine::MainEngine(QObject *parent)
: QQmlApplicationEngine(parent)
, m_viewObject(nullptr)
{
    QObject::connect(this, &MainEngine::objectCreated,
                     this, &MainEngine::_slotObjectCreated);
}

void
MainEngine::_slotObjectCreated(QObject *object, const QUrl &url) {
    if (nullptr == object) {
        qDebug() << "Failed to create the view";
        return;
    }

    // Save the root object
    m_viewObject = object;

    qDebug() << url << " has been loaded. Class name: " << m_viewObject->metaObject()->className();

    // Set the User Agent
    auto objects = m_viewObject->findChildren<QObject *>("webProfile", Qt::FindChildrenRecursively);
    if (0 == objects.length()) {
        qWarning() << "Failed to find webProfile";
        return;
    }
    auto webProfile = objects[0];
    {
        QQmlProperty property(webProfile, "httpUserAgent");
        qDebug() << "UA string: " << property.read().toString();
        property.write(QString(UA_IPHONE_16E));
    }

    // Check the name of the web profile
    {
        QQmlProperty property(webProfile, "storageName");
        qDebug() << "Storage name: " << property.read().toString();
    }

    // Load the correct URL
    objects = m_viewObject->findChildren<QObject *>("webView", Qt::FindChildrenRecursively);
    if (0 == objects.length()) {
        qWarning() << "Failed to find webView";
        return;
    }
    auto webView = objects[0];
    // First stop the loading of whatever is there
    QMetaObject::invokeMethod(webView, "stop");
    // Then load the URL property
    {
        QQmlProperty property(webView, "url");
        property.write(QUrl(URL_GV_MAINPAGE));
    }
}