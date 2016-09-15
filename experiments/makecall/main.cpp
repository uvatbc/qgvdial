#include <QtCore>
#include <QtDBus>
#include <iostream>
using namespace std;

int
main (int argc, char **argv)
{
    if (argc < 1) {
        cerr << "Not enough parameters" << endl;
        return 1;
    }

    QCoreApplication app(argc, argv);
    if (!QDBusConnection::sessionBus().isConnected()) {
        cerr << "Cannot connect to the D-Bus session bus." << endl;
        return 1;
    }

    QDBusInterface iface("org.QGVDial.CallServer",
                         "/org/QGVDial/CallServer",
                         "",
                         QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        cerr << "QGVDial interface is not ready" << endl;
        return 1;
    }

    iface.call("Call", argv[1]);

    qDebug () << QDBusConnection::sessionBus().lastError().message();
    return 1;
}//main
