/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

import Qt 4.7

Item {
    id: container
    objectName: "ProxySettingsPage"

    height: mainColumn.height + 2

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

    property bool bEnableProxy: proxySupport.check
    property bool bSystemProxy: proxySystem.check
    property bool bProxyUserPass: proxyUserPassRequired.check

    Column {
        id: mainColumn

        anchors {
            top: parent.top
            left: parent.left
        }
        spacing: 2
        width: parent.width

        RadioButton {
            id: proxySupport
            width: parent.width

            text: "Enable proxy support"
        }// RadioButton (proxySupport)

        RadioButton {
            id: proxySystem
            width: parent.width
            opacity: (bEnableProxy? 1 : 0)

            text: "Use system proxy settings"
        }// RadioButton (proxySystem)

        Row {
            id: rowUserProxyHost

            height: lblHost.height
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            Text {
                id: lblHost
                text: "Host:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font { family: "Nokia Sans"; pointSize: (10 * g_fontMul) }
            }

            MyTextEdit {
                id: textUserProxyHost
                width: parent.width - lblHost.width
                anchors.verticalCenter: parent.verticalCenter
                text: "proxy.example.com"
                KeyNavigation.tab: textUserProxyPort
                KeyNavigation.backtab: (bEnableProxy && !bSystemProxy && bProxyUserPass ? textUserProxyPass : textUserProxyPort)
            }
        }// Row (user proxy host)

        Row {
            id: rowUserProxyPort

            height: lblPort.height
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            Text {
                id: lblPort
                text: "Port:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font { family: "Nokia Sans"; pointSize: (10 * g_fontMul) }
            }

            MyTextEdit {
                id: textUserProxyPort
                width: parent.width - lblPort.width
                anchors.verticalCenter: parent.verticalCenter
                text: "80"
                validator: IntValidator { bottom: 0; top: 65535 }
                KeyNavigation.tab: (bEnableProxy && !bSystemProxy && bProxyUserPass ? textUserProxyUser : textUserProxyHost)
                KeyNavigation.backtab: textUserProxyHost
            }
        }// Row (user proxy port)

        RadioButton {
            id: proxyUserPassRequired
            width: parent.width
            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            text: "Requires username and password"
        }// RadioButton (proxyUserPassRequired)

        Row {
            id: rowProxyUsername

            height: lblProxyUser.height
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy && bProxyUserPass ? 1 : 0)

            Text {
                id: lblProxyUser
                text: "Proxy user:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font { family: "Nokia Sans"; pointSize: (10 * g_fontMul) }
            }

            MyTextEdit {
                id: textUserProxyUser
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - lblProxyUser.width
                text: "exampleuser"
                KeyNavigation.tab: textUserProxyPass
                KeyNavigation.backtab: textUserProxyPort
            }
        }// Row (user proxy user name)

        Row {
            id: rowProxyPassword

            height: lblProxyPass.height
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy && bProxyUserPass ? 1 : 0)

            Text {
                id: lblProxyPass
                text: "Proxy password:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font { family: "Nokia Sans"; pointSize: (10 * g_fontMul) }
            }

            MyTextEdit {
                id: textUserProxyPass
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - lblProxyPass.width
                text: "hunter2 :P"
                echoMode: TextInput.Password
                KeyNavigation.tab: textUserProxyHost
                KeyNavigation.backtab: textUserProxyUser
            }
        }// Row (user proxy password)

        SaveCancel {
            anchors {
                left: parent.left
                leftMargin: 1
            }
            width: parent.width - 1

            onSigSave: {
                container.sigProxyChanges (bEnableProxy,
                                           bSystemProxy,
                                           textUserProxyHost.text,
                                           textUserProxyPort.text,
                                           bProxyUserPass,
                                           textUserProxyUser.text,
                                           textUserProxyPass.text);
                container.sigDone(true);
            }

            onSigCancel: container.sigDone(false);
        }// Save and cancel buttons
    }// Column
}// Item (top level)
