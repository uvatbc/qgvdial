#include "MainApp.h"
#include "MainWindow.h"
#include "SingletonFactory.h"

int
main(int argc, char *argv[])
{
    MainApp app(argc, argv);
    app.setQuitOnLastWindowClosed (false);

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.init ();

    UniqueAppHelper &unique = Singletons::getRef().getUAH ();
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
