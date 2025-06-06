#include "mainwindow.h"

#include <QTimer>
#include <QStateMachine>

#include <QVBoxLayout>
#include <QLineEdit>

#include <QWebEngineView>
#include <QWebEngineProfile>

#define URL_GV_MAINPAGE "https://voice.google.com"
#define UA_ANDROID_NORD_N200 "Mozilla/5.0 (Linux; Android 12; DE2118) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/112.0.0.0 Mobile Safari/537.36"
#define UA_IPHONE_16E        "Mozilla/5.0 (iPhone17,5; CPU iPhone OS 18_3_2 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Mobile/15E148 FireKeepers/1.7.0"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent) {
    // Set up the main window
    setWindowTitle("Google Voice Login and Call");
    resize(360, 640); // Mobile-like dimensions

    initWebview();
}

void
MainWindow::initLoginSM() {
    auto loginSM = new QStateMachine(this);
}

void
MainWindow::initWebview() {
    // Create a widget and layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    // Create a URL bar
    auto urlBar = new QLineEdit(this);
    layout->addWidget(urlBar);
    urlBar->setReadOnly(true);
    urlBar->setEnabled(false);

    // Create a default web profile
    p_webProfile = new QWebEngineProfile("Default", this);

    // Set up the web view
    p_webView = new QWebEngineView(p_webProfile, this);
    layout->addWidget(p_webView);

    // Set persistent cookie policy: Allow it!
    p_webProfile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);
    // Set user agent to the Android OnePlus Nord N200 device string
    p_webProfile->setHttpUserAgent(UA_ANDROID_NORD_N200);

    qDebug() << "Storage name: " << p_webProfile->storageName();
    qDebug() << "Storage path: " << p_webProfile->persistentStoragePath();
    qDebug() << "Persistent cookie policy: " << p_webProfile->persistentCookiesPolicy();
    qDebug() << "User agent: " << p_webProfile->httpUserAgent();

    // Connect page load finished signal
    QObject::connect(p_webView, &QWebEngineView::loadFinished, this, &MainWindow::handleLoadFinished);
    // Connect page url change signal to the slot that updates the URL bar
    QObject::connect(p_webView, &QWebEngineView::urlChanged, this, &MainWindow::_slotUrlUpdated);
    QObject::connect(this, &MainWindow::_sigUrlUpdated, urlBar, &QLineEdit::setText);

    // Load the Google Voice mobile login page
    p_webView->load(QUrl(URL_GV_MAINPAGE));
}

void
MainWindow::_slotUrlUpdated(const QUrl &url) {
    emit this->_sigUrlUpdated(url.toDisplayString());
}

void
MainWindow::handleLoadFinished(bool ok) {
    if (!ok) {
        qDebug() << "Failed to load page";
        return;
    }

    auto page = p_webView->page();
    qDebug() << "Storage name: " << p_webProfile->storageName();
    qDebug() << "Storage path: " << p_webProfile->persistentStoragePath();
    qDebug() << "Persistent cookie policy: " << p_webProfile->persistentCookiesPolicy();
    qDebug() << "User agent: " << p_webProfile->httpUserAgent();

    // Check if we're on the Google Voice main page after login
    if (p_webView->url().toString().contains("voice.google.com/u/"))
    {
        qDebug("Starting a phone call");
        //initiatePhoneCall();
        return;
    }

    // JavaScript to handle Google login steps
    QString username = "yuvraaj@gmail.com"; // Replace with actual email
    QString password = "oaprvdzeogayxnix";  // Replace with actual password

    QString jsCode = QString(R"(
        (function() {
            // Step 1: Enter email
            let emailInput = document.querySelector('input[type="email"]');
            if (emailInput) {
                emailInput.value = '%1';
                emailInput.dispatchEvent(new Event('input', { bubbles: true }));
                let nextButton = document.querySelector('button[jsname="LgbsSe"]') || 
                                document.querySelector('button[type="button"]');
                if (nextButton) {
                    nextButton.click();
                    return 'email_entered';
                }
                return 'email_input_not_found';
            }

            // Step 2: Enter password (runs on password page)
            let passwordInput = document.querySelector('input[type="password"]');
            if (passwordInput) {
                passwordInput.value = '%2';
                passwordInput.dispatchEvent(new Event('input', { bubbles: true }));
                let submitButton = document.querySelector('button[jsname="LgbsSe"]') || 
                                  document.querySelector('button[type="button"]');
                if (submitButton) {
                    submitButton.click();
                    return 'password_entered';
                }
                return 'password_input_not_found';
            }

            return 'no_input_found';
        })();
    )")
    .arg(username, password);

    // Execute JavaScript and handle result
    page->runJavaScript(jsCode, [](const QVariant &result)
                        { qDebug() << "JavaScript result:" << result.toString(); });

    // Poll for page changes (Google login redirects multiple times)
    QTimer::singleShot(2000, this, &MainWindow::handleLoadFinishedWithOk);
}

void
MainWindow::initiatePhoneCall() {
    // Phone number to call (replace with desired number)
    QString phoneNumber = "+1234567890"; // Replace with actual phone number

    // JavaScript to enter phone number and initiate call
    QString jsCallCode = QString(R"(
        (function() {
            // Find the phone number input field
            let phoneInput = document.querySelector('input[aria-label="Enter a phone number"]') || 
                            document.querySelector('input[type="tel"]');
            if (phoneInput) {
                phoneInput.value = '%1';
                phoneInput.dispatchEvent(new Event('input', { bubbles: true }));
                // Find and click the call button
                let callButton = document.querySelector('button[aria-label="Call"]') || 
                                document.querySelector('button[gv-test-id="make-call-button"]');
                if (callButton) {
                    callButton.click();
                    return 'call_initiated';
                }
                return 'call_button_not_found';
            }
            return 'phone_input_not_found';
        })();
    )").arg(phoneNumber);

    // Execute JavaScript to make the call
    p_webView->page()->runJavaScript(jsCallCode, [](const QVariant &result) {
        qDebug() << "Call JavaScript result:" << result.toString();
    });
}
