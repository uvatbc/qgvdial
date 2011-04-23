#include "MainWindow.h"
#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent /*= 0*/, Qt::WindowFlags flags /*= 0*/)
: QMainWindow(parent, flags)
{
    initLogging ();
}//MainWindow::MainWindow

void
MainWindow::initLogging ()
{
    QString strLogfile = QDir::homePath();
    strLogfile += QDir::separator();
    strLogfile += ".qgvdial";
    strLogfile += QDir::separator();
    strLogfile += "notify.log";
    fLogfile.setFileName (strLogfile);
    fLogfile.open (QIODevice::WriteOnly | QIODevice::Append);
}//MainWindow::initLogging

/** Log information to console and to log file
 * This function is invoked from the qDebug handler that is installed in main.
 * @param strText Text to be logged
 * @param level Log level
 */
void
MainWindow::log (const QString &strText, int level /*= 10*/)
{
    QString strDisp;
    QRegExp regex("^\"(.*)\"\\s*");
    if (strText.indexOf (regex) != -1) {
        strDisp = regex.cap (1);
    } else {
        strDisp = strText;
    }

    QDateTime dt = QDateTime::currentDateTime ();
    QString strLog = QString("%1 : %2 : %3")
                     .arg(dt.toString ("yyyy-MM-dd hh:mm:ss.zzz"))
                     .arg(level)
                     .arg(strDisp);

    // Send to standard output
    cout << strLog.toStdString () << endl;

    // Send to log file
    if (fLogfile.isOpen ()) {
        QTextStream streamLog(&fLogfile);
        streamLog << strLog << endl;
    }
}//MainWindow::log
