#include "mainwindow.h"
#include "NwReqTracker.h"

#define GV_LOGIN_1 "https://accounts.google.com/ServiceLogin?nui=5&service=grandcentral&ltmpl=mobile&btmpl=mobile&passive=true&continue=https://www.google.com/voice/m"
#define GV_ACCOUNT_SERVICELOGIN "https://accounts.google.com/ServiceLogin"
#define GV_ACCOUNT_SMSAUTH      "https://accounts.google.com/SmsAuth"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
, plainText(NULL)
, strUser("yuvraaj@gmail.com")
, nwMgr(this)
, jar(this)
, logsMutex(QMutex::Recursive)
, logsTimer(this)
{
    bool rv;

    plainText = new QPlainTextEdit(this);

    QWidget *central = new QWidget(this);
    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget (plainText, 0,0);

    central->setLayout (layout);
    this->setCentralWidget (central);

    QMenuBar *menuBar = this->menuBar ();
    QMenu *mnuFile = menuBar->addMenu ("&File");
    QAction *actDoIt = mnuFile->addAction ("Do it");
    QAction *actExit = mnuFile->addAction ("E&xit");

    actDoIt->setShortcut (QKeySequence("Ctrl+D"));
    actExit->setShortcut (QKeySequence("Ctrl+Q"));

    rv = connect(actDoIt, SIGNAL(triggered()), this, SLOT(on_actionDo_it()));
    Q_ASSERT(rv);
    rv = connect(actExit, SIGNAL(triggered()), this, SLOT(on_actionExit()));
    Q_ASSERT(rv);

    nwMgr.setCookieJar(&jar);

    rv = connect(&logsTimer, SIGNAL(timeout()), this, SLOT(onLogsTimer()));
    Q_ASSERT(rv);

    QNetworkProxy httpProxy, httpsProxy;
    getSystemProxies (httpProxy, httpsProxy);
    QNetworkProxy::setApplicationProxy (httpProxy);

    logsTimer.setSingleShot (false);
    logsTimer.setInterval (3000);;
    logsTimer.start ();

    QTimer::singleShot (1000, this, SLOT(on_actionDo_it()));
}//MainWindow::MainWindow

MainWindow::~MainWindow()
{
    if (plainText) {
        delete plainText;
        plainText = NULL;
    }
}//MainWindow::~MainWindow

void MainWindow::setOrientation(ScreenOrientation orientation)
{
#if defined(Q_OS_SYMBIAN)
    // If the version of Qt on the device is < 4.7.2, that attribute won't work
    if (orientation != ScreenOrientationAuto) {
        const QStringList v = QString::fromAscii(qVersion()).split(QLatin1Char('.'));
        if (v.count() == 3 && (v.at(0).toInt() << 16 | v.at(1).toInt() << 8 | v.at(2).toInt()) < 0x040702) {
            qWarning("Screen orientation locking only supported with Qt 4.7.2 and above");
            return;
        }
    }
#endif // Q_OS_SYMBIAN

    Qt::WidgetAttribute attribute;
    switch (orientation) {
#if QT_VERSION < 0x040702
    // Qt < 4.7.2 does not yet have the Qt::WA_*Orientation attributes
    case ScreenOrientationLockPortrait:
        attribute = static_cast<Qt::WidgetAttribute>(128);
        break;
    case ScreenOrientationLockLandscape:
        attribute = static_cast<Qt::WidgetAttribute>(129);
        break;
    default:
    case ScreenOrientationAuto:
        attribute = static_cast<Qt::WidgetAttribute>(130);
        break;
#else // QT_VERSION < 0x040702
    case ScreenOrientationLockPortrait:
        attribute = Qt::WA_LockPortraitOrientation;
        break;
    case ScreenOrientationLockLandscape:
        attribute = Qt::WA_LockLandscapeOrientation;
        break;
    default:
    case ScreenOrientationAuto:
        attribute = Qt::WA_AutoOrientation;
        break;
#endif // QT_VERSION < 0x040702
    };
    setAttribute(attribute, true);
}

void MainWindow::showExpanded()
{
#if defined(Q_OS_SYMBIAN) || defined(Q_WS_SIMULATOR)
//    showFullScreen();
    show();
#elif defined(Q_WS_MAEMO_5)
    showMaximized();
#else
    show();
#endif
}

void
MainWindow::log(const QString &strLog)
{
    QMutexLocker locker(&logsMutex);
    logsList.append (strLog);
}//MainWindow::log

void
MainWindow::onLogsTimer()
{
    if (!plainText) {
        return;
    }

    QMutexLocker locker(&logsMutex);
    foreach (QString strLog, logsList) {
        plainText->appendPlainText(strLog);
    }
    logsList.clear ();

    logsTimer.start ();
}//MainWindow::onLogsTimer

void
MainWindow::on_actionExit()
{
    qApp->quit ();
}//MainWindow::on_actionExit

void
MainWindow::on_actionDo_it()
{
    strUser = QInputDialog::getText(this, "Get User", "Enter user",
                                    QLineEdit::Normal, strUser);
    strPass = QInputDialog::getText(this, "Password", "Enter pass",
                                    QLineEdit::Password, strPass);

    // First, HTTP GET the Service login page. This loads up the cookies.

    QUrl url(GV_ACCOUNT_SERVICELOGIN);
    url.addQueryItem("nui"      , "5");
    url.addQueryItem("service"  , "grandcentral");
    url.addQueryItem("ltmpl"    , "mobile");
    url.addQueryItem("btmpl"    , "mobile");
    url.addQueryItem("passive"  , "true");
    url.addQueryItem("continue" , "https://www.google.com/voice/m");

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", UA_IPHONE4);
    NwReqTracker *tracker = new NwReqTracker(nwMgr.get(req), this);

    bool rv = connect(tracker, SIGNAL(sigDone(bool,const QByteArray &)),
                      this   , SLOT (onLogin1(bool,const QByteArray &)));
    Q_ASSERT(rv);
}//MainWindow::on_actionDo_it

bool
MainWindow::getSystemProxies (QNetworkProxy &http, QNetworkProxy &https)
{
#if !DIABLO_OS
    QNetworkProxyFactory::setUseSystemConfiguration (true);
#endif

    do { // Begin cleanup block (not a loop)
        QList<QNetworkProxy> netProxies =
        QNetworkProxyFactory::systemProxyForQuery (
        QNetworkProxyQuery(QUrl("http://www.google.com")));
        http = netProxies[0];
        if (QNetworkProxy::NoProxy != http.type ()) {
            Q_DEBUG("Got proxy: host = ") << http.hostName ()
                           << ", port = " << http.port ();
            break;
        }

        // Otherwise Confirm it
#if defined(Q_WS_X11)
        QString strHttpProxy = getenv ("http_proxy");
        if (strHttpProxy.isEmpty ()) {
            break;
        }

        int colon = strHttpProxy.lastIndexOf (':');
        if (-1 != colon) {
            QString strHost = strHttpProxy.mid (0, colon);
            QString strPort = strHttpProxy.mid (colon);

            strHost.remove ("http://").remove ("https://");

            strPort.remove (':').remove ('/');
            int port = strPort.toInt ();

            Q_DEBUG("Found http proxy :") << strHost << ":" << port;
            http.setHostName (strHost);
            http.setPort (port);
            http.setType (QNetworkProxy::HttpProxy);
        }
#endif
    } while (0); // End cleanup block (not a loop)

    do { // Begin cleanup block (not a loop)
        QList<QNetworkProxy> netProxies =
        QNetworkProxyFactory::systemProxyForQuery (
        QNetworkProxyQuery(QUrl("https://www.google.com")));
        https = netProxies[0];
        if (QNetworkProxy::NoProxy != https.type ()) {
            Q_DEBUG("Got proxy: host =") << https.hostName () << ", port = "
                                         << https.port ();
            break;
        }

        // Otherwise Confirm it
#if defined(Q_WS_X11)
        QString strHttpProxy = getenv ("https_proxy");
        if (strHttpProxy.isEmpty ()) {
            break;
        }

        int colon = strHttpProxy.lastIndexOf (':');
        if (-1 != colon) {
            QString strHost = strHttpProxy.mid (0, colon);
            QString strPort = strHttpProxy.mid (colon);

            strHost.remove ("http://").remove ("https://");

            strPort.remove (':').remove ('/');
            int port = strPort.toInt ();

            Q_DEBUG("Found http proxy: ") << strHost << ":" << port;
            https.setHostName (strHost);
            https.setPort (port);
            https.setType (QNetworkProxy::HttpProxy);
        }
#endif
    } while (0); // End cleanup block (not a loop)

    return (true);
}//MainWindow::getSystemProxies

QString
MainWindow::hasMoved(const QString &strResponse)
{
    QString rv;
    do { // Begin cleanup block (not a loop)
        if (!strResponse.contains ("Moved Temporarily")) {
            break;
        }

        QRegExp rx("a\\s+href=\"(.*)\"\\>", Qt::CaseInsensitive);
        rx.setMinimal (true);
        if (!strResponse.contains (rx) || (rx.captureCount () != 1)) {
            break;
        }

        rv = rx.cap(1);
        Q_DEBUG("Moved temporarily to") << rv;
    } while (0); // End cleanup block (not a loop)

    return rv;
}//MainWindow::hasMoved

void
MainWindow::onLogin1(bool success,const QByteArray & response)
{
    do { // Begin cleanup block (not a loop)
        if (!success) break;

        QString strResponse = response;
        Q_DEBUG(strResponse);

        if (!parseHiddenLoginFields (strResponse, hiddenLoginFields)) {
            break;
        }

        postLogin (GV_ACCOUNT_SERVICELOGIN);
    } while (0); // End cleanup block (not a loop)
}//MainWindow::onLogin1

bool
MainWindow::parseHiddenLoginFields(const QString &strResponse, QVariantMap &ret)
{
/* To match:
  <input type="hidden" name="continue" id="continue"
           value="https://www.google.com/voice/m" />
*/
    QRegExp rx1("\\<input\\s*type\\s*=\\s*\"hidden\"(.*)\\>");
    rx1.setMinimal (true);
    if (!strResponse.contains (rx1)) {
        Q_WARN("Invalid login page!");
        return false;
    }

    ret.clear ();
    int pos = 0;
    while ((pos = rx1.indexIn (strResponse, pos)) != -1) {
        QString fullMatch = rx1.cap(0);
        QString oneInstance = rx1.cap(1);
        QString name, value;
        QRegExp rx2("\"(.*)\""), rx3("'(.*)'");
        rx2.setMinimal (true);
        rx3.setMinimal (true);

        int pos1 = oneInstance.indexOf ("value");
        if (pos1 == -1) {
            goto gonext;
        }

        name  = oneInstance.left (pos1);
        value = oneInstance.mid (pos1);

        if (rx2.indexIn (name) == -1) {
            goto gonext;
        }
        name = rx2.cap (1);

        if (rx2.indexIn (value) == -1) {
            if (rx3.indexIn (value) == -1) {
                goto gonext;
            } else {
                value = rx3.cap (1);
            }
        } else {
            value = rx2.cap (1);
        }

        Q_DEBUG(name) << "=" << value;

        ret[name] = value;

gonext:
        pos += fullMatch.indexOf (oneInstance);
    }

    if (ret.count() == 0) {
        Q_WARN("Invalid login page!");
        return false;
    }

    Q_DEBUG("login fields =") << ret;

    return true;
}//MainWindow::parseHiddenLoginFields

bool
MainWindow::postLogin(QString strUrl)
{
    QUrl url(strUrl);
    QNetworkCookie galx;
    bool found = false;

    foreach (QNetworkCookie cookie, jar.getAllCookies ()) {
        if (cookie.name () == "GALX") {
            galx = cookie;
            found = true;
        }
    }

    if (!found) return false;

    // HTTPS POST the user credentials along with the cookie values as post data

    QStringList keys;
    QVariantMap allLoginFields;
    allLoginFields["passive"]     = "true";
    allLoginFields["timeStmp"]    = "";
    allLoginFields["secTok"]      = "";
    allLoginFields["GALX"]        = galx.value ();
    allLoginFields["Email"]       = strUser;
    allLoginFields["Passwd"]      = strPass;
    allLoginFields["PersistentCookie"] = "yes";
    allLoginFields["rmShown"]     = "1";
    allLoginFields["signIn"]      = "Sign+in";

    keys = hiddenLoginFields.keys();
    foreach (QString key, keys) {
        allLoginFields[key] = hiddenLoginFields[key];
    }

    keys = allLoginFields.keys();
    foreach (QString key, keys) {
        if (key != "dsh") {
            url.addQueryItem(key, allLoginFields[key].toString());
        }
    }

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", UA_IPHONE4);
    req.setHeader (QNetworkRequest::ContentTypeHeader,
                   "application/x-www-form-urlencoded");
    QNetworkReply *reply = nwMgr.post(req, url.encodedQuery ());
    NwReqTracker *tracker = new NwReqTracker(reply, this);

    found = connect(tracker, SIGNAL(sigDone(bool,const QByteArray &)),
                    this   , SLOT (onLogin2(bool,const QByteArray &)));
    Q_ASSERT(found);

    return found;
}//MainWindow::postLogin

void
MainWindow::onLogin2(bool success, const QByteArray &response)
{
    bool bLoggedin = false;
    do { // Begin cleanup block (not a loop)
        if (!success) break;

        QString strMoved;
        QString strResponse = response;

        // There will be 2-3 moved temporarily redirects.
        strMoved = hasMoved(strResponse);
        if (!strMoved.isEmpty ()) {
            if (strMoved.contains ("smsAuth", Qt::CaseInsensitive)) {
                Q_DEBUG("Sms auth!!");
                beginTwoFactorAuth (strMoved);
                break;
            }
            postLogin (strMoved);
            break;
        }

        Q_DEBUG("And we're back!");

        // After this we should have completed login. Check for coolie "gvx"
        foreach (QNetworkCookie cookie, jar.getAllCookies ()) {
            if (cookie.name () == "gvx") {
                bLoggedin = true;
                break;
            }
        }

        // If "gvx" was found, then we're logged in.
        if (!bLoggedin) {
            Q_WARN("Login failed!") << strResponse;
            break;
        }

        getRnr ();
    } while (0); // End cleanup block (not a loop)
}//MainWindow::onLogin2

bool
MainWindow::beginTwoFactorAuth(const QString &strUrl)
{
    QUrl url(strUrl);
    url.addQueryItem("service", "grandcentral");

    QNetworkRequest req(url);
    QNetworkReply *reply = nwMgr.get(req);
    NwReqTracker *tracker = new NwReqTracker(reply, this);

    bool rv =
    connect(tracker, SIGNAL(sigDone(bool,const QByteArray &)),
            this   , SLOT  (onTwoFactorLogin(bool,const QByteArray &)));
    Q_ASSERT(rv);

    return rv;
}//MainWindow::beginTwoFactorAuth

void
MainWindow::onTwoFactorLogin(bool success, const QByteArray &response)
{
    do { // Begin cleanup block (not a loop)
        if (!success) break;

        QString strResponse = response;
        QString strMoved = hasMoved (strResponse);
        if (!strMoved.isEmpty ()) {
            beginTwoFactorAuth (strMoved);
            break;
        }

        Q_DEBUG(strResponse);

        // After which we should have completed login. Check for coolie "gvx"
        success = false;
        foreach (QNetworkCookie gvx, jar.getAllCookies ()) {
            if (gvx.name () == "gvx") {
                success = true;
                break;
            }
        }

        if (success) {
            Q_DEBUG("Login succeeded");
            break;
        }

        success = doTwoFactorAuth (strResponse);
    } while (0); // End cleanup block (not a loop)
}//MainWindow::onTwoFactorLogin

bool
MainWindow::doTwoFactorAuth(const QString &strResponse)
{
    QNetworkCookie galx;
    bool foundgalx = false;
    bool rv = false;

    do { // Begin cleanup block (not a loop)
        foreach (QNetworkCookie cookie, jar.getAllCookies ()) {
            if (cookie.name () == "GALX") {
                galx = cookie;
                foundgalx = true;
            }
        }

        QVariantMap ret;
        if (!parseHiddenLoginFields (strResponse, ret)) {
            break;
        }

        if (!ret.contains ("smsToken")) {
            // It isn't two factor authentication
            Q_WARN("Username or password is incorrect!");
            break;
        }

        if (!foundgalx) {
            Q_WARN("Cannot proceed with two factor auth. Giving up");
            break;
        }

        QString smsUserPin =
        QInputDialog::getText(this, "Two factor authentication",
                              "Enter token", QLineEdit::Normal);
        if (smsUserPin.isEmpty ()) {
            Q_WARN("User didn't enter user pin");
            break;
        }

        QUrl url(GV_ACCOUNT_SMSAUTH), url1(GV_ACCOUNT_SMSAUTH);
        url.addQueryItem("service"  , "grandcentral");

        url1.addQueryItem("smsUserPin"      , smsUserPin);
        url1.addQueryItem("smsVerifyPin"    , "Verify");
        url1.addQueryItem("PersistentCookie", "yes");
        url1.addQueryItem("service"         , "grandcentral");
        url1.addQueryItem("GALX"            , galx.value());

        QStringList keys = ret.keys ();
        foreach (QString key, keys) {
            url1.addQueryItem(key, ret[key].toString());
        }

        Q_DEBUG(QVariant(url).toString());
        Q_DEBUG(QString(url1.encodedQuery()));

        QNetworkRequest req(url);
        req.setRawHeader("User-Agent", UA_IPHONE4);
        req.setHeader (QNetworkRequest::ContentTypeHeader,
                       "application/x-www-form-urlencoded");
        QNetworkReply *reply = nwMgr.post(req, url1.encodedQuery ());
        NwReqTracker *tracker = new NwReqTracker(reply, this);

        rv = connect(tracker, SIGNAL(sigDone(bool,const QByteArray &)),
                     this   , SLOT  (onTwoFactorAutoPost(bool,const QByteArray &)));
        Q_ASSERT(rv);
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//MainWindow::doTwoFactorAuth

void
MainWindow::onTwoFactorAutoPost(bool success, const QByteArray &response)
{
    do { // Begin cleanup block (not a loop)
        if (!success) break;

        QString strResponse = response;
        Q_DEBUG(strResponse);

        QRegExp rx("<form\\s*action\\s*=\\s*\"(.*)\"\\s*method\\s*=\\s*\"POST\"");
        if ((rx.indexIn (strResponse) == -1) || (rx.numCaptures () != 1)) {
            Q_WARN("Failed to login.");
            break;
        }

        QString nextUrl = rx.cap(1);

        QVariantMap ret;
        if (!parseHiddenLoginFields (strResponse, ret)) {
            Q_WARN("Failed to login.");
            break;
        }

        QStringList keys;
        keys = ret.keys();
        foreach (QString key, keys) {
            hiddenLoginFields[key] = ret[key];
        }

        postLogin (nextUrl);
    } while (0); // End cleanup block (not a loop)
}//MainWindow::onTwoFactorAutoPost

bool
MainWindow::getRnr()
{
    QNetworkRequest req(QUrl("https://www.google.com/voice/m/i/all"));
    NwReqTracker *tracker = new NwReqTracker(nwMgr.get(req), this);

    bool rv =
    connect(tracker, SIGNAL(sigDone(bool,const QByteArray &)),
            this   , SLOT  (onGotRnr(bool,const QByteArray &)));
    Q_ASSERT(rv);

    return rv;
}//MainWindow::getRnr

void
MainWindow::onGotRnr(bool success, const QByteArray &response)
{
    QString strResponse = response;

    do { // Begin cleanup block (not a loop)
        if (!success) break;

        QString strMoved = hasMoved (strResponse);
        if (!strMoved.isEmpty ()) {
            QNetworkRequest req(strMoved);
            NwReqTracker *tracker = new NwReqTracker(nwMgr.get(req), this);
            success =
            connect(tracker, SIGNAL(sigDone(bool,const QByteArray &)),
                    this   , SLOT  (onGotRnr(bool,const QByteArray &)));
            Q_ASSERT(success);
            break;
        }

        success = false;
        int pos = strResponse.indexOf ("_rnr_se");
        if (pos == -1) {
            break;
        }

        int pos1 = strResponse.indexOf (">", pos);
        if (pos1 == -1) {
            break;
        }

        QString searchIn = strResponse.mid (pos, pos1-pos);
        QRegExp rx("value\\s*=\\s*\\\"(.*)\\\"");

        if (rx.indexIn (searchIn) == -1) {
            break;
        }

        QString rnr_se = rx.cap (1);
        Q_DEBUG(rnr_se);

        success = true;
    } while (0); // End cleanup block (not a loop)

    if (!success) {
        Q_DEBUG("Did not get rnr_se");
        Q_DEBUG(strResponse);
    }
}//MainWindow::onGotRnr
