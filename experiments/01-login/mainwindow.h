#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "global.h"
#include <QObject>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum ScreenOrientation {
        ScreenOrientationLockPortrait,
        ScreenOrientationLockLandscape,
        ScreenOrientationAuto
    };

    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

    // Note that this will only have an effect on Symbian and Fremantle.
    void setOrientation(ScreenOrientation orientation);
    void showExpanded();

    void log(const QString &strLog);

private:
    QString hasMoved(const QString &strResponse);

    bool postLogin(QString strUrl);
    bool parseHiddenLoginFields(const QString &strResponse, QVariantMap &ret);
    bool getSystemProxies (QNetworkProxy &http, QNetworkProxy &https);

    bool beginTwoFactorAuth(const QString &strUrl);
    bool doTwoFactorAuth(const QString &strResponse);
    bool getRnr();

private slots:
    void onLogsTimer();
    void on_actionExit();
    void on_actionDo_it();

    void onLogin1(bool success, const QByteArray &response);
    void onLogin2(bool success, const QByteArray &response);
    void onTwoFactorLogin(bool success, const QByteArray &response);
    void onTwoFactorAutoPost(bool success, const QByteArray &response);
    void onGotRnr(bool success, const QByteArray &response);

private:
    QPlainTextEdit *plainText;
    QString strUser, strPass;

    QVariantMap hiddenLoginFields;

    QNetworkAccessManager nwMgr;
    CookieJar jar;

    QMutex      logsMutex;
    QStringList logsList;
    QTimer      logsTimer;
};

#endif // MAINWINDOW_H
