import QtQuick 2.15
import QtQuick.Controls 2.15
import QtWebEngine 1.11

ApplicationWindow {
    visible: true
    width: 360
    height: 640
    //title: "Google Voice"

    WebEngineView {
        objectName: "webView"
        anchors.fill: parent
        //url: "https://voice.google.com"
        //url: "https://yuvraaj.net"

        profile: WebEngineProfile {
            objectName: "webProfile"
            storageName: "Default"
            persistentCookiesPolicy: WebEngineProfile.AllowPersistentCookies
            offTheRecord: false
        }

        onLoadingChanged: function(loadRequest) {
            if (loadRequest.status === WebEngineView.LoadSucceeded) {
                console.log(url + " loaded 1");
            } else {
                console.log(url + " loaded. Status = " + loadRequest.status);
            }
        }

        onJavaScriptConsoleMessage: function(level, message, lineNumber, sourceID) {
            console.log("JS Console: " + message + " at line " + lineNumber);
        }
    }
}
