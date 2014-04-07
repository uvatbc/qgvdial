#include "CQmlViewer.h"
#include <QtDeclarative>

CQmlViewer *
createQmlViewer()
{
    return new CQmlViewer;
}

CQmlViewer::CQmlViewer()
{
    connect(this, SIGNAL(statusChanged(QDeclarativeView::Status)),
            this, SLOT(onDeclStatusChanged(QDeclarativeView::Status)));
}//CQmlViewer::CQmlViewer

void
CQmlViewer::onDeclStatusChanged(QDeclarativeView::Status status)
{
    if (QDeclarativeView::Ready == status) {
        emit viewerStatusChanged (true);
        return;
    }

    if (QDeclarativeView::Error != status) {
        return;
    }

    Q_WARN(QString("status = %1").arg (status));
    emit viewerStatusChanged (false);
}//CQmlViewer::onDeclStatusChanged

QDeclarativeContext *
CQmlViewer::rootContext() const
{
    return this->engine()->rootContext();
}//CQmlViewer::rootContext
