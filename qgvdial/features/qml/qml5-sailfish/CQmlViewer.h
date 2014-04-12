#ifndef _CQMLVIEWER_
#define _CQMLVIEWER_

#include "global.h"
#include <QtQuick>

class QmlApplicationViewer {
public:
    enum ScreenOrientation {
        ScreenOrientationAuto
    };
};

class CQmlViewer : public QObject
{
    Q_OBJECT

public:
    CQmlViewer();

    // Stupid hacks to shut up the warnings
    inline void setMainQmlFile(const QString & /*qmlFile*/) {}
    inline void setOrientation(QmlApplicationViewer::ScreenOrientation /*o*/) {}

    // Stub off these show functions.
    inline void show() { m_view->show(); }
    inline void showExpanded() { show(); }

    // Root object and context need to be provided
    inline QQuickItem *rootObject();
    inline QQmlContext *rootContext() const;

    bool connectToChangeNotify(QObject *item, const QString &propName,
                               QObject *receiver, const char *slotName);

signals:
    void viewerStatusChanged(bool ready);

private slots:
    void onDeclStatusChanged(QQuickView::Status status);

private:
    QQuickView *m_view;
};

CQmlViewer *createQmlViewer();

#endif//_CQMLVIEWER_

