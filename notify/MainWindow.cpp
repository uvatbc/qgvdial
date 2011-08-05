#include "MainWindow.h"
#include "NotifySingletons.h"
#include "MqPublisher.h"
#include <iostream>
using namespace std;

MainWindow::MainWindow(QObject *parent /*= 0*/)
: QObject(parent)
, bIsLoggedIn (false)
, oContacts (this)
, oInbox (this)
, checkCounter (0)
{
    initLogging ();

    qRegisterMetaType<ContactInfo>("ContactInfo");

    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    // webPage status
    QObject::connect (&webPage, SIGNAL (status(const QString &, int)),
                       this   , SLOT   (setStatus(const QString &, int)));
    // Status from contacts object
    QObject::connect (&oContacts, SIGNAL (status   (const QString &, int)),
                       this     , SLOT   (setStatus(const QString &, int)));
    // oContacts.allContacts -> this.getContactsDone
    QObject::connect (&oContacts, SIGNAL (allContacts (bool, bool)),
                      this      , SLOT   (getContactsDone (bool, bool)));
    // Status from inbox object
    QObject::connect (&oInbox, SIGNAL (status   (const QString &, int)),
                       this  , SLOT   (setStatus(const QString &, int)));
    // Inbox has updated
    QObject::connect (&oInbox, SIGNAL (inboxChanged ()),
                       this  , SLOT   (inboxChanged ()));
    // Timer tick
    QObject::connect (&mainTimer, SIGNAL(timeout()), this, SLOT(doWork()));

    webPage.setEmitLog (false);

    if (!checkParams ()) {
        qApp->quit();
        return;
    }

    QDateTime dtNow = QDateTime::currentDateTime ();
    QDateTime dtTomorrow = dtNow.addDays (1);
    dtTomorrow.setTime (QTime(0, 0));
    int sec = dtTomorrow.toTime_t() - dtNow.toTime_t();
    if (sec < 0) sec = 1;
    QTimer::singleShot (sec * 1000, this, SLOT(dailyTimeout()));
    qDebug() << "Daily timer first shot after" << sec << "seconds";

    QTimer::singleShot (100, this, SLOT(doWork ()));
}//MainWindow::MainWindow

void
MainWindow::initLogging ()
{
    QString strLogfile = baseDir ();
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

void
MainWindow::setStatus(const QString &strText, int /*timeout = 3000*/)
{
    qDebug () << strText;
}//MainWindow::setStatus

QString
MainWindow::baseDir()
{
    QString strBasedir = QDir::homePath();
    QDir baseDir(strBasedir);
    if (!baseDir.exists (".qgvdial")) {
        baseDir.mkdir (".qgvdial");
    }
    strBasedir += QDir::separator();
    strBasedir += ".qgvdial";
    return strBasedir;
}//MainWindow::baseDir

void
MainWindow::doWork ()
{
    if (!bIsLoggedIn) {
        doLogin ();
        return;
    }

    checkCounter++;
    if (0 == checkCounter % 100) {
        qDebug() << "Checked" << checkCounter << "times";
    }

    // Get the contacts
    oContacts.refreshContacts ();
    // Get inbox
    oInbox.refresh ();
}//MainWindow::doWork

bool
MainWindow::checkParams ()
{
    bool rv = false, bUseDefaultIni = false;
    QString strIni;
    QStringList args = qApp->arguments ();
    if (args.length () < 2) {
        qWarning ("No ini file specified, using default");
        bUseDefaultIni = true;
    } else {
        QFileInfo fi(args[1]);
        strIni = fi.absoluteFilePath ();

        QString strLogfile = strIni + ".log";
        fLogfile.close ();
        fLogfile.setFileName (strLogfile);
        fLogfile.open (QIODevice::WriteOnly | QIODevice::Append);
    }

    if (bUseDefaultIni) {
        strIni = baseDir ();
        strIni += QDir::separator();
        strIni += "notify.ini";
    }

    qDebug () << "Using ini file at" << strIni;

    bool bUseProxy = false, bUseSystemProxy = false, bProxyAuth = false;
    QString strProxy, strProxyUser, strProxyPass;
    int proxyport;

    do { // Begin cleanup block (not a loop)
        QSettings settings (strIni, QSettings::IniFormat, this);

        QByteArray byD;
        QTextStream in(stdin);
        if (!settings.contains ("user")) {
            qWarning ("Ini file does not contain a username");
            cout << "Enter username:";
            in >> strUser;
            settings.setValue ("user", strUser);
        } else {
            strUser = settings.value ("user").toString ();
        }

        if (!settings.contains ("password")) {
            qWarning ("Ini file does not contain a password");
            cout << "Enter password:";
            in >> strPass;
            cipher (strPass.toLocal8Bit (), byD, true);
            settings.setValue ("password", QString(byD.toHex ()));
        } else {
            byD = settings.value("password").toByteArray();
            cipher (QByteArray::fromHex (byD), byD, false);
            strPass = byD;
        }

        checkTimeout = 0;
        if (settings.contains ("check_timeout")) {
            checkTimeout = settings.value ("check_timeout").toUInt ();
        }
        if (checkTimeout < 5) {
            qWarning () << "Check frequency =" << checkTimeout
                        << "is not valid. Resetting to lowest value = 5";
            checkTimeout = 5;
            settings.setValue ("check_timeout", checkTimeout);
        }

        if (!settings.contains ("mqserver")) {
            qWarning ("Ini file does not contain the mq server hostname");
            cout << "Enter server hostname:";
            in >> m_strMqServer;
            settings.setValue ("mqserver", m_strMqServer);
        } else {
            m_strMqServer = settings.value ("mqserver").toString ();
        }

        if (!settings.contains ("mqport")) {
            qWarning ("Ini file does not contain the mq server port");
            cout << "Enter server port:";
            in >> m_mqPort;
            if (0 == m_mqPort) m_mqPort = 1883;
            settings.setValue ("mqport", m_mqPort);
        } else {
            m_mqPort = settings.value ("mqport").toInt (&rv);
            if (!rv) break;
            if (0 == m_mqPort) m_mqPort = 1883;
            settings.setValue ("mqport", m_mqPort);
            rv = false;
        }

        if (!settings.contains ("mqtopic")) {
            qWarning ("Ini file does not contain the mq topic");
            cout << "Enter topic:";
            in >> m_strMqTopic;
            settings.setValue ("mqtopic", m_strMqTopic);
        } else {
            m_strMqTopic = settings.value ("mqtopic").toString ();
        }

        if (!settings.contains ("useproxy")) {
            settings.setValue ("useproxy", false);
            bUseProxy = false;
        } else {
            bUseProxy = settings.value ("useproxy", false).toBool ();
        }

        if (!settings.contains ("usesystemproxy")) {
            settings.setValue ("usesystemproxy", false);
            bUseSystemProxy = false;
        } else {
            bUseSystemProxy = settings.value ("usesystemproxy", false).toBool();
        }

        if (!settings.contains ("proxyserver")) {
            settings.setValue ("proxyserver", "proxy.example.com");
        } else {
            strProxy = settings.value ("proxyserver", "").toString ();
        }

        if (!settings.contains ("proxyport")) {
            settings.setValue ("proxyport", 80);
            proxyport = 80;
        } else {
            proxyport = settings.value ("proxyport", 80).toInt ();
        }

        if (!settings.contains ("proxyauth")) {
            settings.setValue ("proxyauth", false);
            bProxyAuth = false;
        } else {
            bProxyAuth = settings.value ("proxyauth", false).toBool ();
        }

        if (!settings.contains ("proxyuser")) {
            if (bUseProxy & !bUseSystemProxy && bProxyAuth) {
                qWarning ("Ini file needs proxy authentication user");
                cout << "Enter proxy user:";
                in >> strProxyUser;
                settings.setValue ("proxyuser", "proxy_user");
            }
        } else {
            strProxyUser = settings.value ("proxyuser", "").toString ();
        }

        if (!settings.contains ("proxypass")) {
            if (bUseProxy & !bUseSystemProxy && bProxyAuth) {
                qWarning ("Ini file needs proxy authentication password");
                cout << "Enter proxy password:";
                in >> strProxyPass;
                cipher (strPass.toLocal8Bit (), byD, true);
                settings.setValue ("proxypass", QString(byD.toHex ()));
            }
        } else {
            byD = settings.value("proxypass", "").toByteArray ();
            cipher (QByteArray::fromHex (byD), byD, false);
            strProxyPass = byD;
        }

        if (bUseProxy) {
            GVAccess &webPage = Singletons::getRef().getGVAccess ();
            webPage.setProxySettings (bUseProxy, bUseSystemProxy,
                                      strProxy, proxyport,
                                      bProxyAuth,
                                      strProxyUser, strProxyPass);
        }
        rv = true;
    } while (0); // End cleanup block (not a loop)

    return rv;
}//MainWindow::checkParams

bool
MainWindow::cipher(const QByteArray &byIn, QByteArray &byOut, bool bEncrypt)
{
    int iEVP, inl, outl, c;
    EVP_CIPHER_CTX cipherCtx;
    char cipherIv[16], cipherIn[16], cipherOut[16 + EVP_MAX_BLOCK_LENGTH];
    memset (&cipherCtx, 0, sizeof cipherCtx);
    memset (&cipherIv, 0xFA, sizeof cipherIv);

#define QGV_CIPHER_KEY "01234567890123456789012345678901"
    EVP_CIPHER_CTX_init (&cipherCtx);
    iEVP = EVP_CipherInit_ex (&cipherCtx, EVP_aes_256_cbc (), NULL, NULL, NULL,
                              bEncrypt?1:0);
    if (1 != iEVP) return false;
    EVP_CIPHER_CTX_set_key_length(&cipherCtx, sizeof(QGV_CIPHER_KEY)-1);
    iEVP = EVP_CipherInit_ex (&cipherCtx, EVP_aes_256_cbc (), NULL,
                              (quint8 *) QGV_CIPHER_KEY, (quint8 *) cipherIv,
                               bEncrypt?1:0);
    if (1 != iEVP) return false;

    byOut.clear ();
    c = 0;
    while (c < byIn.size ()) {
        inl = byIn.size () - c;
        inl = (uint)inl > sizeof (cipherIn) ? sizeof (cipherIn) : inl;
        memcpy (cipherIn, &(byIn.constData()[c]), inl);
        memset (&cipherOut, 0, sizeof cipherOut);
        outl = sizeof cipherOut;
        iEVP = EVP_CipherUpdate (&cipherCtx,
                                (quint8 *) &cipherOut, &outl,
                                 (quint8 *) &cipherIn , inl);
        if (1 != iEVP) {
            qWarning ("Cipher update failed. Aborting");
            break;
        }
        byOut += QByteArray(cipherOut, outl);
        c += inl;
    }

    if (1 == iEVP) {
        outl = sizeof cipherOut;
        memset (&cipherOut, 0, sizeof cipherOut);
        iEVP = EVP_CipherFinal_ex (&cipherCtx, (quint8 *) &cipherOut, &outl);
        if (1 == iEVP) {
            byOut += QByteArray(cipherOut, outl);
        }
    }

    EVP_CIPHER_CTX_cleanup(&cipherCtx);

    return (1 == iEVP);
}//MainWindow::cipher

/** Invoked to begin the login process.
 * We already have the username and password, so just start the login to the GV
 * website. The async completion routine is loginCompleted.
 */
void
MainWindow::doLogin ()
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QVariantList l;

    bool bOk = false;
    do { // Begin cleanup block (not a loop)
        webPage.setTimeout(60);

        l += strUser;
        l += strPass;

        qDebug ("Logging in...");

        // webPage.workCompleted -> this.loginCompleted
        if (!webPage.enqueueWork (GVAW_login, l, this,
                SLOT (loginCompleted (bool, const QVariantList &))))
        {
            qCritical ("Login returned immediately with failure!");
            break;
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)

    if (!bOk)
    {
        webPage.setTimeout(20);
        // Cleanup if any
        strUser.clear ();
        strPass.clear ();

        l.clear ();
        logoutCompleted (true, l);

        qApp->quit ();
    }
}//MainWindow::doLogin

void
MainWindow::loginCompleted (bool bOk, const QVariantList & /*varList*/)
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    webPage.setTimeout(20);

    if (!bOk) {
        QVariantList l;
        logoutCompleted (true, l);

        qCritical ("User login failed");
        qApp->quit ();
    } else {
        qDebug ("User logged in");

        bIsLoggedIn = true;

        oContacts.setUserPass (strUser, strPass);
        oContacts.loginSuccess ();
        oInbox.loginSuccess ();

        QTimer::singleShot (100, this, SLOT(doWork ()));
    }
}//MainWindow::loginCompleted

void
MainWindow::doLogout ()
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QVariantList l;
    webPage.enqueueWork (GVAW_logout, l, this,
                         SLOT (logoutCompleted (bool, const QVariantList &)));
}//MainWindow::doLogout

void
MainWindow::logoutCompleted (bool, const QVariantList &)
{
    // This clears out the table and the view as well
    oContacts.loggedOut ();
    oInbox.loggedOut ();

    bIsLoggedIn = false;
}//MainWindow::logoutCompleted

void
MainWindow::getContactsDone (bool bChanges, bool bOK)
{
    if (bOK && bChanges) {
        qDebug ("Contacts changed, update mosquitto");

        MqPublisher pub(QString("qgvnotify:%1").arg(QHostInfo::localHostName()),
                        m_strMqServer, m_mqPort, m_strMqTopic,
                        this);
        pub.publish ("contact");
    }

    startTimer ();
}//MainWindow::getContactsDone

void
MainWindow::inboxChanged ()
{
    qDebug ("Inbox changed, update mosquitto");

    MqPublisher pub(QString("qgvnotify:%1").arg(QHostInfo::localHostName()),
                    m_strMqServer, m_mqPort, m_strMqTopic,
                    this);
    pub.publish ("inbox");

    startTimer ();
}//MainWindow::inboxChanged

void
MainWindow::startTimer ()
{
    mainTimer.stop ();
    mainTimer.setSingleShot (true);
    mainTimer.setInterval (checkTimeout * 1000);
    mainTimer.start ();
}//MainWindow::startTimer

void
MainWindow::dailyTimeout ()
{
    qDebug() << "Daily timer timed out at" << QDateTime::currentDateTime ();
    inboxChanged ();
    QTimer::singleShot (24 * 60 * 60 * 1000, this, SLOT(dailyTimeout()));
}//MainWindow::dailyTimeout
