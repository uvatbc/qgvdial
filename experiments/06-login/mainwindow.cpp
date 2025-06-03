#include "mainwindow.h"

#include <QTimer>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Set up the main window
    setWindowTitle("Google Voice Login and Call");
    resize(360, 640); // Mobile-like dimensions

    // Create a widget and layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    // Set up the web view
    webView = new QWebEngineView(this);
    layout->addWidget(webView);

    // Load the Google Voice mobile login page
    webView->load(QUrl("https://voice.google.com"));

    // Connect page load finished signal
    QObject::connect(webView, &QWebEngineView::loadFinished, this, &MainWindow::handleLoadFinished);
}

void
MainWindow::handleLoadFinished(bool ok) {
    if (!ok) {
        qDebug() << "Failed to load page";
        return;
    }

    // Check if we're on the Google Voice main page after login
    if (webView->url().toString().contains("voice.google.com/u/"))
    {
        initiatePhoneCall();
        return;
    }

    // JavaScript to handle Google login steps
    QString username = "your_email@example.com"; // Replace with actual email
    QString password = "your_password";          // Replace with actual password

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
    webView->page()->runJavaScript(jsCode, [](const QVariant &result)
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
    webView->page()->runJavaScript(jsCallCode, [](const QVariant &result) {
        qDebug() << "Call JavaScript result:" << result.toString();
    });
}
