#include <QtGui/QApplication>
#include "MainWindow.h"
#include "UniqueAppHelper.h"

#if defined (Q_OS_UNIX)
#include <TelepathyQt4/Constants>
#include <TelepathyQt4/Types>
#if !NO_DBGINFO
#include <TelepathyQt4/Debug>
#endif
#endif

int
main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed (false);

#if defined (Q_OS_UNIX)
    Tp::registerTypes();
#if !NO_DBGINFO
    Tp::enableDebug(true);
    Tp::enableWarnings(true);
#endif
#endif

    UniqueAppHelper &unique = UniqueAppHelper::getRef ();
    if (!unique.setUnique ())
    {
        unique.signalOriginal ();
        return (0);
    }

    MainWindow w;
#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    return app.exec();
}//main
