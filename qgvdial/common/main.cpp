#include "MainWindow.h"

Q_DECL_EXPORT int
main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    MainWindow win;
    win.init ();

    return app->exec();
}
