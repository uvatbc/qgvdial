#include <QGuiApplication>
#include <QtWebEngineQuick>
#include "mainengine.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QtWebEngineQuick::initialize();

    MainEngine engine;
    engine.load(QUrl("qrc:/gv/main.qml"));

    return app.exec();
}

//#include "main.moc"
