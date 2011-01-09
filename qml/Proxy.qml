import Qt 4.7
import "helper.js" as Code

Item {
    id: container

    signal sigDone(bool bSave)
    signal sigProxyChanges(bool bEnable,
                           bool bUserSystemSettings,
                           string host, int port,
                           bool bRequiresAuth,
                           string user, string pass)

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
            fontPoint: Code.btnFontPoint()/10

            text: "Enable proxy support"
            onCheckChanged: container.sigProxySupport(check)
        }// RadioButton (proxySupport)

        RadioButton {
            id: proxySystem
            width: parent.width
            opacity: (bEnableProxy? 1 : 0)
            fontPoint: Code.btnFontPoint()/10

            text: "Use system proxy settings"
            onCheckChanged: container.sigUseSystemProxy(check)
        }// RadioButton (proxySystem)

        Row {
            width: parent.width
            spacing: 2
            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            Text {
                text: "Host:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: Code.btnFontPoint()/10
            }

            TextInput {
                id: textUserProxyHost
                anchors.verticalCenter: parent.verticalCenter
                text: "Enter proxy host"
                color: "white"
                font.pointSize: Code.btnFontPoint()/10
            }
        }// Row (user proxy host)

        Row {
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            Text {
                text: "Port:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: Code.btnFontPoint()/10
            }

            TextInput {
                id: textUserProxyPort
                anchors.verticalCenter: parent.verticalCenter
                text: "Enter proxy port"
                color: "white"
                validator: IntValidator { bottom: 0; top: 65535 }
                font.pointSize: Code.btnFontPoint()/10
            }
        }// Row (user proxy port)

        RadioButton {
            id: proxyUserPassRequired
            width: parent.width
            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)
            fontPoint: Code.btnFontPoint()/10

            text: "Requires username and password"
            onCheckChanged: container.sigUseSystemProxy(check)
        }// RadioButton (proxyUserPassRequired)

        Row {
            width: parent.width
            spacing: 2
            opacity: (bEnableProxy && !bSystemProxy && bProxyUserPass ? 1 : 0)

            Text {
                text: "Proxy user:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: Code.btnFontPoint()/10
            }

            TextInput {
                id: textUserProxyUser
                anchors.verticalCenter: parent.verticalCenter
                text: ""
                color: "white"
                font.pointSize: Code.btnFontPoint()/10
            }
        }// Row (user proxy user name)

        Row {
            width: parent.width
            spacing: 2
            opacity: (bEnableProxy && !bSystemProxy && bProxyUserPass ? 1 : 0)

            Text {
                text: "Proxy password:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: Code.btnFontPoint()/10
            }

            TextInput {
                id: textUserProxyPass
                anchors.verticalCenter: parent.verticalCenter
                text: ""
                color: "white"
                font.pointSize: Code.btnFontPoint()/10
            }
        }// Row (user proxy password)

        Row {
            width: parent.width
            spacing: 1

            MyButton {
                mainText: "Save"
                width: (parent.width / 2) - parent.spacing
                mainFontPoint: Code.btnFontPoint()/10

                onClicked: {
                    container.sigProxyChanges (bEnableProxy,
                                               bSystemProxy,
                                               textUserProxyHost,
                                               textUserProxyPort,
                                               bProxyUserPass,
                                               textUserProxyUser,
                                               textUserProxyPass);
                    container.sigDone(true);
                }

            }//MyButton (Save)

            MyButton {
                mainText: "Cancel"
                width: (parent.width / 2) - parent.spacing
                mainFontPoint: Code.btnFontPoint()/10

                onClicked: container.sigDone(false);
            }//MyButton (Cancel)
        }
    }// Column
}// Item (top level)
