#include <QtGui/QApplication>
#include "qmlapplicationviewer.h"
#include "MainWindow.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    QmlApplicationViewer viewer;
    MainWindow win(&viewer);
    win.init ();

    return app->exec();
}
