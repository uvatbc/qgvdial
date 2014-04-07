#include "CQmlViewer.h"
#include <sailfishapp.h>

CQmlViewer *
createQmlViewer()
{
    return new CQmlViewer;
}

CQmlViewer::CQmlViewer()
: m_view(SailfishApp::createView())
{
    connect(m_view, SIGNAL(statusChanged(QQuickView::Status)),
            this, SLOT(onDeclStatusChanged(QQuickView::Status)));
}//CQmlViewer::CQmlViewer

void
CQmlViewer::onDeclStatusChanged(QQuickView::Status status)
{
    if (QQuickView::Ready == status) {
        emit viewerStatusChanged (true);
        return;
    }

    if (QQuickView::Error != status) {
        return;
    }

    Q_WARN(QString("status = %1").arg (status));
    emit viewerStatusChanged (false);
}//CQmlViewer::onDeclStatusChanged

QQuickItem *
CQmlViewer::rootObject()
{
    return m_view->rootObject();
}//CQmlViewer::rootObject

QQmlContext *
CQmlViewer::rootContext() const
{
    return m_view->rootContext ();
}//CQmlViewer::rootContext
