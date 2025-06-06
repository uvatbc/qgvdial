#pragma once

#include <QMainWindow>

class QWebEngineProfile;
class QWebEngineView;
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void handleLoadFinished(bool ok);
    void handleLoadFinishedWithOk() {handleLoadFinished(true);}
    void initiatePhoneCall();


/** Private signals/slots within this object only **/
signals:
    void _sigUrlUpdated(const QString &url);
private slots:
    void _slotUrlUpdated(const QUrl &url);
/***************************************************/

private:
    void initLoginSM();
    void initWebview();

private:
    QWebEngineProfile *p_webProfile;
    QWebEngineView *p_webView;
};
