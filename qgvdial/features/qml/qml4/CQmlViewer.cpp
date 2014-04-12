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

bool
CQmlViewer::connectToChangeNotify(QObject *item, const QString &propName,
                                  QObject *receiver, const char *slotName)
{
    bool rv = false;
    do {
        const QMetaObject *metaObject = item->metaObject ();
        if (NULL == metaObject) {
            Q_WARN("NULL metaObject");
            break;
        }

        QMetaProperty metaProp;
        for (int i = 0; i < metaObject->propertyCount (); i++) {
            metaProp = metaObject->property (i);
            if (metaProp.name () == propName) {
                rv = true;
                break;
            }
        }

        if (!rv) {
            Q_WARN(QString("Couldn't find property named %1").arg(propName));
            break;
        }
        rv = false;

        if (!metaProp.hasNotifySignal ()) {
            Q_WARN(QString("Property %1 does not have a notify signal")
                   .arg(propName));
            break;
        }

        QString signalName = QString("2%1")
                                .arg(metaProp.notifySignal().signature());

        Q_DEBUG(QString("Connect %1 to %2").arg(signalName).arg(slotName));

        rv =
        connect(item, signalName.toLatin1().constData(), receiver, slotName);
    } while(0);

    return (rv);
}//CQmlViewer::connectToChangeNotify
