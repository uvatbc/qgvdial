#include <QtGui/QApplication>
#include "MainWindow.h"
#include "UniqueAppHelper.h"
#include "SingletonFactory.h"

int
main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed (false);

    OsDependent &osd = SingletonFactory::getRef().getOSD ();
    osd.init ();

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
