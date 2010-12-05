/*
 qgv-util is derived from vicar-utils and is licensed under GNU GPL.
 */

/*
@version: 0.5
@author: Sudheer K. <scifi1947 at gmail.com>
@license: GNU General Public License
*/

#include "telepathyutility.h"
#include <QtCore>
#include <QtDBus>

#include <iostream>
#include <fstream>
using namespace std;

ofstream logfile;

void
open_logfile ()
{
    if (!logfile.is_open ()) {
        logfile.open("/home/user/.qgvdial/qgv-util.log", ios::app);
    }
}

void dbgHandler(QtMsgType type, const char *msg)
{
    QDateTime dt = QDateTime::currentDateTime ();
    int level = 0;

    switch (type) {
        case QtDebugMsg:
            level = 3;
            break;
        case QtWarningMsg:
            level = 2;
            break;
        case QtCriticalMsg:
            level = 1;
            break;
        case QtFatalMsg:
            level = 0;
            break;
    }

    QString strLog = QString("%1 : %2 : %3")
                     .arg(dt.toString ("yyyy-MM-dd hh:mm:ss.zzz"))
                     .arg(level)
                     .arg(msg);
    open_logfile ();
    logfile << strLog.toAscii().data() << endl;
    cout << strLog.toAscii().data() << endl;

    if (QtFatalMsg == type) {
        abort ();
    }
}

int
main(int argc, char *argv[])
{
    open_logfile ();
    qInstallMsgHandler(dbgHandler);

    qDBusRegisterMetaType<org::freedesktop::Telepathy::SimplePresence>();
    //qDBusRegisterMetaType<org::maemo::vicar::Profile>();
    //qDBusRegisterMetaType<org::maemo::vicar::ProfileList>();

    TelepathyUtility *tpUtility = new TelepathyUtility();

    if (argc > 1 && argv[1]) {
        QString instruction = QString(argv[1]);
        if (instruction == "INSTALL") {
            //Check if Account already exists
            if (!tpUtility->accountExists()) {
                qDebug() << "qgv-tp account not found. Creating ..";
                bool result = tpUtility->createAccount();
                if (!result) exit(1);
            } else {
                qDebug() << "qgv-tp account found.";
            }
        } else if (instruction == "REMOVE") {
            bool result = tpUtility->deleteAccount();
            if (!result) exit(2);
        } else if (instruction == "ACCOUNTSTATUS") {
            QString status = tpUtility->getAccountStatus();
            qDebug() << "Account Status is "<< status;
        }
    }

    delete (tpUtility);
    tpUtility = 0;

    logfile.close();
    return 0;
}
