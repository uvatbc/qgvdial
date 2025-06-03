#pragma once

#include <QWebEngineView>
#include <QMainWindow>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void handleLoadFinished(bool ok);
    void handleLoadFinishedWithOk() {handleLoadFinished(true);}
    void initiatePhoneCall();

private:
    QWebEngineView *webView;
};
