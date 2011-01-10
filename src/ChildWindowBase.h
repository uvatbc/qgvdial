#ifndef CHILDWINDOWBASE_H
#define CHILDWINDOWBASE_H

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

#ifdef Q_WS_MAEMO_5
typedef QWidget ChildWindowBaseClass;
#define ChildWindowBase_flags (Qt::Window)
#else
typedef QDialog ChildWindowBaseClass;
#define ChildWindowBase_flags (0)
#endif

class ChildWindowBase : public ChildWindowBaseClass
{
    Q_OBJECT
public:
    ChildWindowBase (QWidget *parent    = 0,
                     Qt::WindowFlags  f = 0);

#if defined (Q_WS_MAEMO_5)
protected slots:
    virtual void done (int) {}
#endif

signals:
    //! Status emitter
    void status(const QString &strText, int timeout = 2000);
};

#endif // CHILDWINDOWBASE_H
