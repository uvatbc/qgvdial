/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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

import QtQuick 1.1
import com.nokia.meego 1.1

Item {
    id: container
    objectName: "ProxySettingsPage"

    height: mainColumn.height + 2

    function setValues(bEnable, bUseSystemProxy, host, port,
                       bRequiresAuth, user, pass) {
        console.debug ("QML: Setting proxy settings")
        proxySupport.checked = bEnable;
        proxySystem.checked = bUseSystemProxy;
        textUserProxyHost.text = host;
        textUserProxyPort.text = port;
        proxyUserPassRequired.checked = bRequiresAuth;
        textUserProxyUser.text = user;
        textUserProxyPass.text = pass;
    }

    signal sigRevertChanges
    signal sigProxyChanges(bool bEnable,
                           bool bUseSystemProxy,
                           string host, int port,
                           bool bRequiresAuth,
                           string user, string pass)

    property bool bEnableProxy: proxySupport.checked
    property bool bSystemProxy: proxySystem.checked
    property bool bProxyUserPass: proxyUserPassRequired.checked

    Column {
        id: mainColumn

        anchors {
            top: parent.top
            left: parent.left
        }
        spacing: 5
        width: parent.width

        CheckBox {
            id: proxySupport
            width: parent.width - 20
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: 10
                rightMargin: 10
            }

            text: "Enable proxy support"
        }// CheckBox (proxySupport)

        CheckBox {
            id: proxySystem
            width: parent.width - 20
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: 10
                rightMargin: 10
            }

            opacity: (bEnableProxy? 1 : 0)

            text: "Use system proxy settings"
        }// CheckBox (proxySystem)

        Row {
            id: rowUserProxyHost

            height: lblHost.height > textUserProxyHost.height ? lblHost.height : textUserProxyHost.height
            width: parent.width - 20
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: 10
                rightMargin: 10
            }

            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            Label {
                id: lblHost
                text: "Host:"
                anchors.verticalCenter: parent.verticalCenter
            }//Label ("Host:")

            TextField  {
                id: textUserProxyHost
                width: parent.width - lblHost.width
                anchors.verticalCenter: parent.verticalCenter
                placeholderText: "Proxy server"
                KeyNavigation.tab: textUserProxyPort
                KeyNavigation.backtab: (bEnableProxy && !bSystemProxy && bProxyUserPass ? textUserProxyPass : textUserProxyPort)
            }//TextField (proxy host)
        }// Row (user proxy host)

        Row {
            id: rowUserProxyPort

            height: lblPort.height > textUserProxyPort.height ? lblPort.height : textUserProxyPort.height
            width: parent.width - 20
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: 10
                rightMargin: 10
            }

            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            Label {
                id: lblPort
                text: "Port:"
                anchors.verticalCenter: parent.verticalCenter
            }//Label ("Port:")

            TextField {
                id: textUserProxyPort
                width: parent.width - lblPort.width
                anchors.verticalCenter: parent.verticalCenter
                placeholderText: "Proxy port"
                validator: IntValidator { bottom: 0; top: 65535 }
                inputMethodHints: Qt.ImhDialableCharactersOnly
                KeyNavigation.tab: (bEnableProxy && !bSystemProxy && bProxyUserPass ? textUserProxyUser : textUserProxyHost)
                KeyNavigation.backtab: textUserProxyHost
            }//TextField (proxy port)
        }// Row (user proxy port)

        CheckBox {
            id: proxyUserPassRequired
            width: parent.width - 20
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: 10
                rightMargin: 10
            }

            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            text: "Requires user and pass"
        }// CheckBox (proxyUserPassRequired)

        Row {
            id: rowProxyUsername

            height: lblProxyUser.height > textUserProxyUser.height ? lblProxyUser.height : textUserProxyUser.height
            width: parent.width - 20
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: 10
                rightMargin: 10
            }

            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy && bProxyUserPass ? 1 : 0)

            Label {
                id: lblProxyUser
                text: "User:"
                anchors.verticalCenter: parent.verticalCenter
            }//Label ("Proxy user:")

            TextField {
                id: textUserProxyUser
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - lblProxyUser.width
                placeholderText: "Proxy user"
                KeyNavigation.tab: textUserProxyPass
                KeyNavigation.backtab: textUserProxyPort
            }//TextField (proxy user)
        }// Row (user proxy user name)

        Row {
            id: rowProxyPassword

            height: lblProxyPass.height > textUserProxyPass.height ? lblProxyPass.height : textUserProxyPass.height
            width: parent.width - 20
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: 10
                rightMargin: 10
            }

            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy && bProxyUserPass ? 1 : 0)

            Label {
                id: lblProxyPass
                text: "Password:"
                anchors.verticalCenter: parent.verticalCenter
            }//Label ("Password:")

            TextField {
                id: textUserProxyPass
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - lblProxyPass.width
                placeholderText: "Proxy password"
                echoMode: TextInput.Password
                KeyNavigation.tab: textUserProxyHost
                KeyNavigation.backtab: textUserProxyUser
            }//TextField (proxy password)
        }// Row (user proxy password)

        ButtonRow {
            exclusive: false
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                text: "Revert"
                onClicked: container.sigRevertChanges(false);
            }
            Button {
                text: "Submit"

                onClicked: {
                    container.sigProxyChanges (bEnableProxy,
                                               bSystemProxy,
                                               textUserProxyHost.text,
                                               textUserProxyPort.text,
                                               bProxyUserPass,
                                               textUserProxyUser.text,
                                               textUserProxyPass.text);
                }
            }
        }
    }// Column
}// Item (top level)
