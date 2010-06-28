#ifndef CHILDWINDOWBASE_H
#define CHILDWINDOWBASE_H

#include <QtGui>

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

signals:
    //! Log emitter
    void log(const QString &strText, int level = 10);
    //! Status emitter
    void status(const QString &strText, int timeout = 2000);
};

#endif // CHILDWINDOWBASE_H
