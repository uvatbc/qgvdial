import Qt 4.7
import "helper.js" as Code

Item {
    id: container
    objectName: "ProxySettingsPage"

    function setValues(bEnable, bUseSystemProxy, host, port,
                       bRequiresAuth, user, pass) {
        console.debug ("QML: Setting proxy settings")
        proxySupport.check = bEnable;
        proxySystem.check = bUseSystemProxy;
        textUserProxyHost.text = host;
        textUserProxyPort.text = port;
        proxyUserPassRequired.check = bRequiresAuth;
        textUserProxyUser.text = user;
        textUserProxyPass.text = pass;
    }

    signal sigDone(bool bSave)
    signal sigProxyChanges(bool bEnable,
                           bool bUseSystemProxy,
                           string host, int port,
                           bool bRequiresAuth,
                           string user, string pass)

    signal sigMsgBoxDone (bool ok)

    property bool bEnableProxy: proxySupport.check
    property bool bSystemProxy: proxySystem.check
    property bool bProxyUserPass: proxyUserPassRequired.check

    Column {
        anchors.fill: parent
        anchors.topMargin: 2
        spacing: 2

        RadioButton {
            id: proxySupport
            width: parent.width
            pixelSize: (container.height + container.width) / 30

            text: "Enable proxy support"
        }// RadioButton (proxySupport)

        RadioButton {
            id: proxySystem
            width: parent.width
            opacity: (bEnableProxy? 1 : 0)
            pixelSize: (container.height + container.width) / 30

            text: "Use system proxy settings"
        }// RadioButton (proxySystem)

        Row {
            width: parent.width
            spacing: 2
            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            Text {
                id: lblHost
                text: "Host:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: (container.height + container.width) / 30
            }

            MyTextEdit {
                id: textUserProxyHost
                width: parent.width - lblHost.width
                anchors.verticalCenter: parent.verticalCenter
                text: "proxy.example.com"
                pixelSize: (container.height + container.width) / 30
                KeyNavigation.tab: textUserProxyPort
                KeyNavigation.backtab: (bEnableProxy && !bSystemProxy && bProxyUserPass ? textUserProxyPass : textUserProxyPort)
            }
        }// Row (user proxy host)

        Row {
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            Text {
                id: lblPort
                text: "Port:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: (container.height + container.width) / 30
            }

            MyTextEdit {
                id: textUserProxyPort
                width: parent.width - lblPort.width
                anchors.verticalCenter: parent.verticalCenter
                text: "80"
                validator: IntValidator { bottom: 0; top: 65535 }
                pixelSize: (container.height + container.width) / 30
                KeyNavigation.tab: (bEnableProxy && !bSystemProxy && bProxyUserPass ? textUserProxyUser : textUserProxyHost)
                KeyNavigation.backtab: textUserProxyHost
            }
        }// Row (user proxy port)

        RadioButton {
            id: proxyUserPassRequired
            width: parent.width
            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)
            pixelSize: (container.height + container.width) / 30

            text: "Requires username and password"
        }// RadioButton (proxyUserPassRequired)

        Row {
            width: parent.width
            spacing: 2
            opacity: (bEnableProxy && !bSystemProxy && bProxyUserPass ? 1 : 0)

            Text {
                id: lblProxyUser
                text: "Proxy user:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: (container.height + container.width) / 30
            }

            MyTextEdit {
                id: textUserProxyUser
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - lblProxyUser.width
                text: "exampleuser"
                pixelSize: (container.height + container.width) / 30
                KeyNavigation.tab: textUserProxyPass
                KeyNavigation.backtab: textUserProxyPort
            }
        }// Row (user proxy user name)

        Row {
            width: parent.width
            spacing: 2
            opacity: (bEnableProxy && !bSystemProxy && bProxyUserPass ? 1 : 0)

            Text {
                id: lblProxyPass
                text: "Proxy password:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: (container.height + container.width) / 30
            }

            MyTextEdit {
                id: textUserProxyPass
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - lblProxyPass.width
                text: "hunter2 :P"
                echoMode: TextInput.Password
                pixelSize: (container.height + container.width) / 30
                KeyNavigation.tab: textUserProxyHost
                KeyNavigation.backtab: textUserProxyUser
            }
        }// Row (user proxy password)

        Row {
            width: parent.width
            spacing: 1

            MyButton {
                mainText: "Save"
                width: (parent.width / 2) - parent.spacing
                mainPixelSize: (container.height + container.width) / 30

                onClicked: {
                    container.sigProxyChanges (bEnableProxy,
                                               bSystemProxy,
                                               textUserProxyHost.text,
                                               textUserProxyPort.text,
                                               bProxyUserPass,
                                               textUserProxyUser.text,
                                               textUserProxyPass.text);
                    container.sigDone(true);
                }

            }//MyButton (Save)

            MyButton {
                mainText: "Cancel"
                width: (parent.width / 2) - parent.spacing
                mainPixelSize: (container.height + container.width) / 30

                onClicked: container.sigDone(false);
            }//MyButton (Cancel)
        }
    }// Column

    MsgBox {
        id: msgBox
        opacity: ((container.opacity == 1 && g_bShowMsg == true) ? 1 : 0)
        msgText: g_strMsgText

        width: container.width - 20
        height: (container.width + container.height) / 6
        anchors.centerIn: container

        onSigMsgBoxOk: container.sigMsgBoxDone(true)
        onSigMsgBoxCancel: container.sigMsgBoxDone(false)
    }

}// Item (top level)
