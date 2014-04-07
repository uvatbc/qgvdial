#ifndef _CQMLVIEWER_
#define _CQMLVIEWER_

#include "global.h"
#include "qmlapplicationviewer.h"

class CQmlViewer : public QmlApplicationViewer
{
    Q_OBJECT

public:
    CQmlViewer();

    QDeclarativeContext *rootContext() const;

signals:
    void viewerStatusChanged(bool ready);

private slots:
    void onDeclStatusChanged(QDeclarativeView::Status status);
};

CQmlViewer *createQmlViewer();

#endif//_CQMLVIEWER_

