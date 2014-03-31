#ifndef _CQMLVIEWER_
#define _CQMLVIEWER_

#include "global.h"

class CQmlViewer : public QObject
{
    Q_OBJECT

public:
    CQmlViewer();

signals:
    void viewerStatusChanged(bool ready);

private slots:
    void onDeclStatusChanged(QQuickView::Status status);

private:
    QQuickView *m_view;
};

#endif//_CQMLVIEWER_

