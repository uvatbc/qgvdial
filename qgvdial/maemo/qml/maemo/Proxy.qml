/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016  Yuvraaj Kelkar

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
    property real internalPointSize: 16

    Column {
        id: mainColumn

        anchors {
            top: parent.top
            left: parent.left
        }
        spacing: 2
        width: parent.width

        QGVRadioButton {
            id: proxySupport
            width: parent.width

            text: "Enable proxy support"
            pointSize: container.internalPointSize
        }// QGVRadioButton (proxySupport)

        QGVRadioButton {
            id: proxySystem
            width: parent.width
            opacity: (bEnableProxy? 1 : 0)

            text: "Use system proxy settings"
            pointSize: container.internalPointSize
        }// QGVRadioButton (proxySystem)

        Row {
            id: rowUserProxyHost

            height: lblHost.height > textUserProxyHost.height ? lblHost.height : textUserProxyHost.height
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            Text {
                id: lblHost
                text: "Host:"
                anchors.verticalCenter: parent.verticalCenter
                color: "white"
            }//QGVLabel ("Host:")

            TextOneLine {
                id: textUserProxyHost
                width: parent.width - lblHost.width
                anchors.verticalCenter: parent.verticalCenter
                placeholderText: "Proxy server"
                font.pointSize: internalPointSize
                KeyNavigation.tab: textUserProxyPort
                KeyNavigation.backtab: (bEnableProxy && !bSystemProxy && bProxyUserPass ? textUserProxyPass : textUserProxyPort)
            }//QGVTextInput (proxy host)
        }// Row (user proxy host)

        Row {
            id: rowUserProxyPort

            height: lblPort.height > textUserProxyPort.height ? lblPort.height : textUserProxyPort.height
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            Text {
                id: lblPort
                text: "Port:"
                color: "white"
                font.pointSize: internalPointSize
                anchors.verticalCenter: parent.verticalCenter
            }//QGVLabel ("Port:")

            TextOneLine {
                id: textUserProxyPort
                width: parent.width - lblPort.width
                anchors.verticalCenter: parent.verticalCenter
                placeholderText: "Proxy port"
                font.pointSize: internalPointSize
                //validator: IntValidator { bottom: 0; top: 65535 }
                KeyNavigation.tab: (bEnableProxy && !bSystemProxy && bProxyUserPass ? textUserProxyUser : textUserProxyHost)
                KeyNavigation.backtab: textUserProxyHost
            }//QGVTextInput (proxy port)
        }// Row (user proxy port)

        QGVRadioButton {
            id: proxyUserPassRequired
            width: parent.width
            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            text: "Requires user and pass"
            pointSize: container.internalPointSize
        }// QGVRadioButton (proxyUserPassRequired)

        Row {
            id: rowProxyUsername

            height: lblProxyUser.height > textUserProxyUser.height ? lblProxyUser.height : textUserProxyUser.height
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy && bProxyUserPass ? 1 : 0)

            Text {
                id: lblProxyUser
                text: "Proxy user:"
                color: "white"
                font.pointSize: internalPointSize
                anchors.verticalCenter: parent.verticalCenter
            }//QGVLabel ("Proxy user:")

            TextOneLine {
                id: textUserProxyUser
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - lblProxyUser.width
                placeholderText: "Proxy Username"
                font.pointSize: internalPointSize
                KeyNavigation.tab: textUserProxyPass
                KeyNavigation.backtab: textUserProxyPort
            }//QGVTextInput (proxy user)
        }// Row (user proxy user name)

        Row {
            id: rowProxyPassword

            height: lblProxyPass.height > textUserProxyPass.height ? lblProxyPass.height : textUserProxyPass.height
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy && bProxyUserPass ? 1 : 0)

            Text {
                id: lblProxyPass
                text: "Proxy password:"
                font.pointSize: internalPointSize
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
            }//QGVLabel ("Proxy password:")

            TextOneLine {
                id: textUserProxyPass
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - lblProxyPass.width
                placeholderText: "Proxy password"
                font.pointSize: internalPointSize
                echoMode: TextInput.Password
                KeyNavigation.tab: textUserProxyHost
                KeyNavigation.backtab: textUserProxyUser
            }//QGVTextInput (proxy password)
        }// Row (user proxy password)

        Row {
            width: parent.width
            spacing: 4

            Button {
                width: (parent.width-parent.spacing) / 2
                text: "Revert"
                onClicked: container.sigRevertChanges(false);
            }

            Button {
                text: "Submit"
                width: (parent.width-parent.spacing) / 2

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
        }//Row of buttons
    }// Column
}// Item (top level)
