#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>

namespace Ui {
class MainWindow;
}

class O2;
class QWebView;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onLinkedChanged();
    void onLinkingFailed();
    void onLinkingSucceeded();
    void onOpenBrowser(const QUrl &url);
    void onCloseBrowser();

private:
    Ui::MainWindow *ui;
    O2  *o2;
    QWebView *webView;
};

#endif // MAINWINDOW_H
