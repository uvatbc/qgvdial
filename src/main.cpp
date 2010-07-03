#include <QtGui/QApplication>
#include "MainWindow.h"
#include "UniqueAppHelper.h"
#include "OsDependent.h"

int
main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed (false);

    OsDependent &osd = OsDependent::getRef ();
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
