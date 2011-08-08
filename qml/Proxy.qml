/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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

    height: mainColumn.height

    property real pixHeight: 500

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
        height: {
            var rv = 2 + proxySupport.height + 2 +
                         rowSaveCancel.height + 6;
            if (bEnableProxy) {
                rv += proxySystem.height + 2;
                if (!bSystemProxy) {
                    rv += rowUserProxyHost.height + 2 +
                          rowUserProxyPort.height + 2 +
                            proxyUserPassRequired.height + 2;
                    if (bProxyUserPass) {
                        rv += rowProxyUsername.height + 2 +
                              rowProxyPassword.height + 2;
                    }
                }
            }
            return rv;
        }

        RadioButton {
            id: proxySupport
            width: parent.width
            pixelSize: pixHeight

            text: "Enable proxy support"
        }// RadioButton (proxySupport)

        RadioButton {
            id: proxySystem
            width: parent.width
            opacity: (bEnableProxy? 1 : 0)
            pixelSize: pixHeight

            text: "Use system proxy settings"
        }// RadioButton (proxySystem)

        Row {
            id: rowUserProxyHost

            height: pixHeight + 2
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            Text {
                id: lblHost
                text: "Host:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: parent.height - 2
            }

            MyTextEdit {
                id: textUserProxyHost
                width: parent.width - lblHost.width
                anchors.verticalCenter: parent.verticalCenter
                text: "proxy.example.com"
                pixelSize: parent.height - 2
                KeyNavigation.tab: textUserProxyPort
                KeyNavigation.backtab: (bEnableProxy && !bSystemProxy && bProxyUserPass ? textUserProxyPass : textUserProxyPort)
            }
        }// Row (user proxy host)

        Row {
            id: rowUserProxyPort

            height: pixHeight + 2
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)

            Text {
                id: lblPort
                text: "Port:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: parent.height - 2
            }

            MyTextEdit {
                id: textUserProxyPort
                width: parent.width - lblPort.width
                anchors.verticalCenter: parent.verticalCenter
                text: "80"
                validator: IntValidator { bottom: 0; top: 65535 }
                pixelSize: parent.height - 2
                KeyNavigation.tab: (bEnableProxy && !bSystemProxy && bProxyUserPass ? textUserProxyUser : textUserProxyHost)
                KeyNavigation.backtab: textUserProxyHost
            }
        }// Row (user proxy port)

        RadioButton {
            id: proxyUserPassRequired
            width: parent.width
            opacity: (bEnableProxy && !bSystemProxy ? 1 : 0)
            pixelSize: pixHeight

            text: "Requires username and password"
        }// RadioButton (proxyUserPassRequired)

        Row {
            id: rowProxyUsername

            height: pixHeight + 2
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy && bProxyUserPass ? 1 : 0)

            Text {
                id: lblProxyUser
                text: "Proxy user:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: parent.height - 2
            }

            MyTextEdit {
                id: textUserProxyUser
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - lblProxyUser.width
                text: "exampleuser"
                pixelSize: parent.height - 2
                KeyNavigation.tab: textUserProxyPass
                KeyNavigation.backtab: textUserProxyPort
            }
        }// Row (user proxy user name)

        Row {
            id: rowProxyPassword

            height: pixHeight + 2
            width: parent.width
            spacing: 2

            opacity: (bEnableProxy && !bSystemProxy && bProxyUserPass ? 1 : 0)

            Text {
                id: lblProxyPass
                text: "Proxy password:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: parent.height - 2
            }

            MyTextEdit {
                id: textUserProxyPass
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - lblProxyPass.width
                text: "hunter2 :P"
                echoMode: TextInput.Password
                pixelSize: parent.height - 2
                KeyNavigation.tab: textUserProxyHost
                KeyNavigation.backtab: textUserProxyUser
            }
        }// Row (user proxy password)

        Row {
            id: rowSaveCancel

            height: pixHeight + 2
            width: parent.width
            spacing: 1

            MyButton {
                mainText: "Save"
                width: (parent.width / 2) - parent.spacing
                mainPixelSize: parent.height - 2

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
                mainPixelSize: parent.height - 2

                onClicked: container.sigDone(false);
            }//MyButton (Cancel)
        }
    }// Column
}// Item (top level)
