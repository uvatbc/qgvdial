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
import "meego"
import "generic"
import "s3"

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
    property real internalPointSize: (8 * g_fontMul)

    onSigDone: {
        textUserProxyHost.closeSoftwareInputPanel();
        textUserProxyPort.closeSoftwareInputPanel();
        textUserProxyUser.closeSoftwareInputPanel();
        textUserProxyPass.closeSoftwareInputPanel();
    }

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
            pointSize: container.internalPointSize
        }// RadioButton (proxySupport)

        RadioButton {
            id: proxySystem
            width: parent.width
            opacity: (bEnableProxy? 1 : 0)

            text: "Use system proxy settings"
            pointSize: container.internalPointSize
        }// RadioButton (proxySystem)

        Row {
            id: rowUserProxyHost

            height: lblHost.height > textUserProxyHost.height ? lblHost.height : textUserProxyHost.height
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            QGVLabel {
                id: lblHost
                text: "Host:"
                anchors.verticalCenter: parent.verticalCenter
            }//QGVLabel ("Host:")

            QGVTextInput {
                id: textUserProxyHost
                width: parent.width - lblHost.width
                anchors.verticalCenter: parent.verticalCenter
                text: "proxy.example.com"
                KeyNavigation.tab: textUserProxyPort
                KeyNavigation.backtab: (bEnableProxy && !bSystemProxy && bProxyUserPass ? textUserProxyPass : textUserProxyPort)
                fontPointMultiplier: 8.0 / 10.0
            }//QGVTextInput (proxy host)
        }// Row (user proxy host)

        Row {
            id: rowUserProxyPort

            height: lblPort.height > textUserProxyPort.height ? lblPort.height : textUserProxyPort.height
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            QGVLabel {
                id: lblPort
                text: "Port:"
                anchors.verticalCenter: parent.verticalCenter
            }//QGVLabel ("Port:")

            QGVTextInput {
                id: textUserProxyPort
                width: parent.width - lblPort.width
                anchors.verticalCenter: parent.verticalCenter
                text: "80"
                validator: IntValidator { bottom: 0; top: 65535 }
                KeyNavigation.tab: (bEnableProxy && !bSystemProxy && bProxyUserPass ? textUserProxyUser : textUserProxyHost)
                KeyNavigation.backtab: textUserProxyHost
                fontPointMultiplier: 8.0 / 10.0
            }//QGVTextInput (proxy port)
        }// Row (user proxy port)

        RadioButton {
            id: proxyUserPassRequired
            width: parent.width
            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            text: "Requires user and pass"
            pointSize: container.internalPointSize
        }// RadioButton (proxyUserPassRequired)

        Row {
            id: rowProxyUsername

            height: lblProxyUser.height > textUserProxyUser.height ? lblProxyUser.height : textUserProxyUser.height
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy && bProxyUserPass ? 1 : 0)

            QGVLabel {
                id: lblProxyUser
                text: "Proxy user:"
                anchors.verticalCenter: parent.verticalCenter
            }//QGVLabel ("Proxy user:")

            QGVTextInput {
                id: textUserProxyUser
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - lblProxyUser.width
                text: "exampleuser"
                KeyNavigation.tab: textUserProxyPass
                KeyNavigation.backtab: textUserProxyPort
                fontPointMultiplier: 8.0 / 10.0
            }//QGVTextInput (proxy user)
        }// Row (user proxy user name)

        Row {
            id: rowProxyPassword

            height: lblProxyPass.height > textUserProxyPass.height ? lblProxyPass.height : textUserProxyPass.height
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy && bProxyUserPass ? 1 : 0)

            QGVLabel {
                id: lblProxyPass
                text: "Proxy password:"
                anchors.verticalCenter: parent.verticalCenter
            }//QGVLabel ("Proxy password:")

            QGVTextInput {
                id: textUserProxyPass
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - lblProxyPass.width
                text: "hunter2 :P"
                echoMode: TextInput.Password
                KeyNavigation.tab: textUserProxyHost
                KeyNavigation.backtab: textUserProxyUser
                fontPointMultiplier: 8.0 / 10.0
            }//QGVTextInput (proxy password)
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
