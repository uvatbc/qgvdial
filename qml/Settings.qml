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

Flickable {
    id: container
    objectName: "SettingsPage"

    signal sigUserChanged(string username)
    signal sigPassChanged(string password)
    signal sigLogin
    signal sigLogout
    signal sigLinkActivated(string strLink)

    signal sigProxyChanges(bool bEnable,
                           bool bUseSystemProxy,
                           string host, int port,
                           bool bRequiresAuth,
                           string user, string pass)
    signal sigProxyRefresh
    signal sigMosquittoChanges(bool bEnable, string host, int port, string topic)
    signal sigMosquittoRefresh
    signal sigPinSettingChanges(bool bEnable, string pin)
    signal sigPinRefresh

    signal sigSendLogs

    property real pixHeight: (height + width) / 30
    property real textItemHeight: pixHeight + 4

    contentHeight: expandLoginDetails.height + expandProxySettings.height +
                   expandMqSettings.height + expandPinSettings.height +
                   expandLogView.height + expandAbout.height
    contentWidth: width
    clip: true

    function setUsername (strU) {
        loginDetails.username = strU;
    }
    function setPassword (strP) {
        loginDetails.password = strP;
    }

    ExpandView {
        id: expandLoginDetails
        anchors {
            top: parent.top
            left: parent.left
        }

        width: parent.width
        contentHeight: loginDetails.height;

        mainTitle: "Login details"
        mainTitlePixHeight: container.pixHeight

        LoginDetails {
            id: loginDetails
            y: parent.startY

            width: parent.width - 1
            pixHeight: container.pixHeight
            opacity: parent.containedOpacity

            onSigUserChanged: container.sigUserChanged(user);
            onSigPassChanged: container.sigPassChanged(pass);
            onSigLogin: container.sigLogin();
            onSigLogout: container.sigLogout();
        }
    }//ExpandView (login/logout)

    ExpandView {
        id: expandProxySettings
        anchors {
            top: expandLoginDetails.bottom
            left: parent.left
        }

        width: parent.width
        contentHeight: proxySettings.height;

        mainTitle: "Proxy"
        mainTitlePixHeight: container.pixHeight

        Proxy {
            id: proxySettings
            y: parent.startY

            width: parent.width - 1
            pixHeight: container.pixHeight

            opacity: parent.containedOpacity

            onSigProxyChanges: container.sigProxyChanges(bEnable, bUseSystemProxy,
                                                         host, port,
                                                         bRequiresAuth,
                                                         user, pass);
            onSigDone: {
                if (!bSave) {
                    container.sigProxyRefresh();
                }
                parent.isExpanded = false;
            }
        }
    }//ExpandView (proxy)

    ExpandView {
        id: expandMqSettings
        anchors {
            top: expandProxySettings.bottom
            left: parent.left
        }

        width: parent.width
        contentHeight: mqSettings.height;

        mainTitle: "Mosquitto"
        mainTitlePixHeight: container.pixHeight

        Mosquitto {
            id: mqSettings
            y: parent.startY

            width: parent.width - 1
            pixHeight: container.pixHeight

            opacity: parent.containedOpacity

            onSigDone: {
                if (!bSave) {
                    container.sigMosquittoRefresh();
                }
                parent.isExpanded = false;
            }
            onSigMosquittoChanges: container.sigMosquittoChanges(bEnable, host, port, topic)
        }//Mosquitto
    }//ExpandView (mosquitto)

    ExpandView {
        id: expandPinSettings
        anchors {
            top: expandMqSettings.bottom
            left: parent.left
        }

        width: parent.width
        contentHeight: pinSettings.height;

        mainTitle: "Pin"
        mainTitlePixHeight: container.pixHeight

        PinSetting {
            id: pinSettings
            y: parent.startY

            width: parent.width - 1
            pixHeight: container.pixHeight

            opacity: parent.containedOpacity

            onSigDone: {
                if (!bSave) {
                    container.sigPinRefresh();
                }
                parent.isExpanded = false;
            }
            onSigPinSettingChanges: container.sigPinSettingChanges(bEnable, pin)
        }//Pin settings
    }//ExpandView (pin settings)

    ExpandView {
        id: expandLogView
        anchors {
            top: expandPinSettings.bottom
            left: parent.left
        }

        width: parent.width
        contentHeight: logView.height;

        mainTitle: "Logs"
        mainTitlePixHeight: container.pixHeight

        LogView {
            id: logView

            width: parent.width - 1
            height: container.height
            y: parent.startY

            opacity: parent.containedOpacity
            onSigBack: parent.isExpanded = false;
            onSigSendLogs: container.sigSendLogs();
        }
    }//ExpandView (log view)

    ExpandView {
        id: expandAbout
        anchors {
            top: expandLogView.bottom
            left: parent.left
        }

        width: parent.width
        contentHeight: aboutWin.height;

        mainTitle: "About"
        mainTitlePixHeight: container.pixHeight

        About {
            id: aboutWin

            width: parent.width - 1
            y: parent.startY
            pixHeight: container.pixHeight
            opacity: parent.containedOpacity

            onSigLinkActivated: container.sigLinkActivated(strLink)
        }//About
    }//ExpandView (About)
}// Item (container)
