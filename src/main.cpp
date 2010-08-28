#include "MainApp.h"
#include "MainWindow.h"
#include "Singletons.h"

int
main (int argc, char *argv[])
{
    MainApp app(argc, argv);
    if (app.isRunning ()) {
        app.sendMessage ("show");
        return (0);
    }

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.init ();

    MainWindow w;
    app.setActivationWindow (&w);
    app.setQuitOnLastWindowClosed (false);

#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    return (app.exec());
}//main
