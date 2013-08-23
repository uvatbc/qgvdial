#include "MainWindow.h"
#include "MqPublisher.h"
#include <iostream>
using namespace std;

// Forward function declarations
QString baseDir();
void initLogging (const QString &userIni);
extern QString strLogfile;

MainWindow::MainWindow(QObject *parent /*= 0*/)
: QObject(parent)
, gvApi (false, this)
, bIsLoggedIn (false)
, oContacts (this)
, oInbox (gvApi, this)
, checkCounter (0)
, client (NULL)
{
    qRegisterMetaType<ContactInfo>("ContactInfo");

    QTimer::singleShot (10, this, SLOT(init()));
}//MainWindow::MainWindow

void
MainWindow::setStatus(const QString &strText, int /*timeout = 3000*/)
{
    Q_DEBUG(strText);
}//MainWindow::setStatus

void
MainWindow::init()
{
    connect(&gvApi, SIGNAL(twoStepAuthentication(AsyncTaskToken*)),
             this , SLOT(onTwoStepAuthentication(AsyncTaskToken*)));

    // Status from contacts object
    QObject::connect (&oContacts, SIGNAL(status(const QString&,int)),
                       this     , SLOT(setStatus(const QString&,int)));
    // oContacts.allContacts -> this.getContactsDone
    QObject::connect (&oContacts, SIGNAL(allContacts(bool,bool)),
                      this      , SLOT(getContactsDone(bool,bool)));
    // Status from inbox object
    QObject::connect (&oInbox, SIGNAL(status(const QString&,int)),
                       this  , SLOT(setStatus(const QString&,int)));
    // Inbox has updated
    QObject::connect (&oInbox, SIGNAL(inboxChanged()),
                       this  , SLOT(inboxChanged()));
    // Timer tick
    QObject::connect (&mainTimer, SIGNAL(timeout()), this, SLOT(doWork()));

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
    Q_DEBUG(QString("Daily timer first shot after %1 seconds").arg (sec));

    client = new QGVNotifyProxyIface("net.yuvraaj.qgvnotify.control", "/",
                                     QDBusConnection::sessionBus(), 0);
    if (NULL == client) {
        Q_CRIT("Failed to initialize DBus client");
        getOut ();
        return;
    }
    connect(client, SIGNAL(CommandForClient(const QString&)),
            this  , SLOT(onCommandForClient(const QString&)));

    onCommandForClient("getUser");

    doWork ();
}//MainWindow::init

void
MainWindow::doWork ()
{
    if (!bIsLoggedIn) {
        doLogin ();
        return;
    }

    checkCounter++;
    if (0 == checkCounter % 100) {
        Q_DEBUG(QString("Checked %1 times").arg (checkCounter));
    }

    // Get the contacts
    oContacts.refreshContacts ();
    // Get inbox
    oInbox.refresh ();
}//MainWindow::doWork

bool
MainWindow::checkParams ()
{
    bool rv = false, bUseDefaultIni = false, tempBool;
    QStringList args = qApp->arguments ();
    if (args.length () < 2) {
        qWarning ("No ini file specified, using default");
        bUseDefaultIni = true;
    } else {
        QFileInfo fi(args[1]);
        strIni = fi.absoluteFilePath ();

        initLogging (strIni);
    }

    if (bUseDefaultIni) {
        strIni = baseDir ();
        strIni += QDir::separator();
        strIni += "notify.ini";
    }

    Q_DEBUG(QString("Using ini file at %1").arg (strIni));

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

        if (!settings.contains ("tfaRequired")) {
            tempBool = false;
            settings.setValue ("tfaRequired", tempBool);
            tfaRequired = false;
        } else {
            tfaRequired = settings.value("tfaRequired").toBool();
        }

        if (tfaRequired) {
            if (!settings.contains ("cpass")) {
                qWarning ("Ini file does not contain an application specific "
                          "password");
                cout << "Enter application specific password:";
                in >> strCPass;
                cipher (strCPass.toLocal8Bit (), byD, true);
                settings.setValue ("cpass", QString(byD.toHex ()));
            } else {
                byD = settings.value("cpass").toByteArray();
                cipher (QByteArray::fromHex (byD), byD, false);
                strCPass = byD;
            }
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
            if (m_strMqTopic.startsWith ('-')) {
                qWarning ("Using gv_notify as topic instead");
                m_strMqTopic = "gv_notify";
            }
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
            gvApi.setProxySettings (bUseProxy, bUseSystemProxy,
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
    bool bOk = false;
    AsyncTaskToken *token = new AsyncTaskToken(this);
    do { // Begin cleanup block (not a loop)
        if (!token) {
            Q_WARN("Failed to allocate token");
            break;
        }

        //loadCookies ();

        bOk = connect(token, SIGNAL(completed()),
                      this , SLOT(loginCompleted()));

        token->inParams["user"] = strUser;
        token->inParams["pass"] = strPass;

        Q_DEBUG("Logging in...");

        // webPage.workCompleted -> this.loginCompleted
        if (!gvApi.login (token)) {
            Q_CRIT("Login returned immediately with failure!");
            break;
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)

    if (!bOk) {
        if (token) {
            token->deleteLater ();
            token = NULL;
        }

        // Cleanup if any
        strUser.clear ();
        strPass.clear ();

        qApp->quit ();
    }
}//MainWindow::doLogin

void
MainWindow::loginCompleted()
{
    AsyncTaskToken *token = (AsyncTaskToken *) QObject::sender ();

    if (!token || (token->status != ATTS_SUCCESS)) {
        Q_CRIT("User login failed");
        qApp->quit ();
    } else {
        Q_DEBUG("User logged in");

        bIsLoggedIn = true;

        if (tfaRequired) {
            Q_DEBUG("TFA");

            if (strCPass.isEmpty ()) {
                QTextStream in(stdin);

                cout << "Enter application specific password:";
                in >> strCPass;
            }

            QByteArray byD;
            QSettings settings (strIni, QSettings::IniFormat, this);

            cipher (strCPass.toLocal8Bit (), byD, true);
            settings.setValue ("cpass", QString(byD.toHex ()));
            settings.setValue ("tfaRequired", tfaRequired);
        } else {
            strCPass = strPass;
        }

        //saveCookies ();

        oContacts.login (strUser, strCPass);
        oInbox.loginSuccess ();

        QTimer::singleShot (100, this, SLOT(doWork ()));
    }

    if (token) {
        delete token;
    }
}//MainWindow::loginCompleted

void
MainWindow::doLogout ()
{
    AsyncTaskToken *token = new AsyncTaskToken(this);
    connect (token, SIGNAL(completed()),
             this, SLOT(logoutCompleted()));

    if (!gvApi.logout (token)) {
        token->deleteLater ();
        return;
    }
}//MainWindow::doLogout

void
MainWindow::logoutCompleted ()
{
    AsyncTaskToken *token = (AsyncTaskToken *) QObject::sender ();

    // This clears out the table and the view as well
    oContacts.logout ();
    oInbox.loggedOut ();

    bIsLoggedIn = false;

    if (token) {
        token->deleteLater ();
    }
}//MainWindow::logoutCompleted

void
MainWindow::onTwoStepAuthentication(AsyncTaskToken *token)
{
    QTextStream in(stdin);

    qWarning ("Two step authentication PIN required.");

    QString strPIN;
    cout << "Enter PIN:";
    in >> strPIN;

    if (strPIN.startsWith ('-')) {
        qWarning ("Requesting GV Api to call you:");
        gvApi.resumeTFAAltLogin (token);
    } else {
        token->inParams["user_pin"] = strPIN;
        tfaRequired = true;
        gvApi.resumeTFALogin (token);
    }
}//MainWindow::onTwoStepAuthentication

void
MainWindow::getContactsDone (bool bChanges, bool bOK)
{
    if (bOK && bChanges) {
        Q_DEBUG("Contacts changed, update mosquitto");

        MqPublisher pub(QString("qgvnotify:%1").arg(QHostInfo::localHostName()),
                        m_strMqServer, m_mqPort, m_strMqTopic,
                        this);
        pub.publish(QString("contact %1")
                    .arg(QDateTime::currentDateTime().toUTC().toTime_t())
                    .toAscii());
    }

    startTimer ();
}//MainWindow::getContactsDone

void
MainWindow::inboxChanged ()
{
    Q_DEBUG("Inbox changed, update mosquitto");

    MqPublisher pub(QString("qgvnotify:%1").arg(QHostInfo::localHostName()),
                    m_strMqServer, m_mqPort, m_strMqTopic,
                    this);
    pub.publish(QString("inbox %1")
                .arg(QDateTime::currentDateTime().toUTC().toTime_t())
                .toAscii());

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
    Q_DEBUG(QString("Daily timer timed out at %1")
                .arg (QDateTime::currentDateTime().toString()));
    inboxChanged ();
    QTimer::singleShot (24 * 60 * 60 * 1000, this, SLOT(dailyTimeout()));
}//MainWindow::dailyTimeout

void
MainWindow::getOut()
{
    Q_DEBUG("quit!!");
    qApp->quit ();
}//MainWindow::getOut

void
MainWindow::onCommandForClient(const QString &command)
{
    Q_DEBUG(QString("command = %1").arg(command));

    if (NULL == client) {
        Q_WARN("client is NULL");
        return;
    }

    if (command == "getUser") {
        client->ReportUser(strUser, strIni, strLogfile,
                           qApp->applicationPid());
    } else if (command == "quitAll") {
        getOut ();
    }
}//MainWindow::onCommandForClient

void
MainWindow::loadCookies()
{
    QList<QNetworkCookie> cookies;
    QSettings settings (strIni, QSettings::IniFormat, this);
    QByteArray name, value;
    quint32 expiration;
    int i;

    int size = settings.beginReadArray ("cookies");
    for (i = 0; i < size; i++) {
        name       = settings.value("name").toByteArray();
        value      = settings.value("value").toByteArray();
        expiration = settings.value("expiration").toUInt();

        QNetworkCookie cookie(name, value);
        cookie.setDomain(settings.value("domain").toString());

        if (settings.value("isSession").toBool()) {
            cookie.setExpirationDate (QDateTime::fromTime_t (expiration));
        }

        cookie.setHttpOnly(settings.value("isHttpOnly").toBool());
        cookie.setSecure(settings.value("isSecure").toBool());
        cookie.setPath(settings.value("path").toString());

        cookies.append (cookie);
    }
    settings.endArray ();

    gvApi.setAllCookies (cookies);

    Q_DEBUG(QString("Loaded %1 cookies").arg(i));
}//MainWindow::loadCookies

void
MainWindow::saveCookies()
{
    QList<QNetworkCookie> cookies = gvApi.getAllCookies ();
    QSettings settings (strIni, QSettings::IniFormat, this);
    int i;

    settings.remove ("cookies");
    settings.beginWriteArray ("cookies");

    for (i = 0; i < cookies.count(); i++) {
        settings.setArrayIndex (i);
        settings.setValue ("name",       cookies.at(i).name());
        settings.setValue ("value",      cookies.at(i).value());
        settings.setValue ("expiration", cookies.at(i).expirationDate().toTime_t());
        settings.setValue ("domain",     cookies.at(i).domain());
        settings.setValue ("isSession",  cookies.at(i).isSessionCookie());
        settings.setValue ("isSecure",   cookies.at(i).isSecure());
        settings.setValue ("isHttpOnly", cookies.at(i).isHttpOnly());
        settings.setValue ("path",       cookies.at(i).path());
    }

    settings.endArray ();

    Q_DEBUG(QString("Saved %1 cookies").arg(i));
}//MainWindow::saveCookies
