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

import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: container

    signal sigLinkActivated(string strLink)

    signal sigMosquittoChanges(bool bEnable, string host, int port, string topic)
    signal sigMosquittoRefresh
    signal sigPinSettingChanges(bool bEnable, string pin)
    signal sigPinRefresh

    signal sigSendLogs

    Timer {
        id: yAdjustTimer

        property real setY: 0.0
        onSetYChanged: {
            yAdjustTimer.stop();
            yAdjustTimer.start();
        }

        interval: 300; repeat: false; running: false
        //onTriggered: container.contentY = setY;
    }//Timer for delayed setting of contentY so that when you click on an
    // ExpandView, it gets into focus. Especially important for the logs

    SilicaFlickable {
        // Cannot use childrenRect.height because it causes a binding loop
        contentHeight: (expandLoginDetails.height + 1 +
                        expandProxySettings.height + 1 +
                        expandUpdateSettings.height + 1 +
                        expandEtCetera.height)
        contentWidth: width
        clip: true

        anchors.fill: parent

        ExpandView {
            id: expandLoginDetails
            objectName: "ExpandLoginDetails"

            anchors {
                top: parent.top
                left: parent.left
            }
            width: parent.width

            mainTitle: "Login details"
            content: loginDetails
            yTimer: yAdjustTimer

            LoginDetails {
                id: loginDetails
                width: parent.width - 1
                visible: parent.isExpanded
            }
        }//ExpandView (login/logout)

        ExpandView {
            id: expandProxySettings
            anchors {
                top: expandLoginDetails.bottom
                left: parent.left
                topMargin: 1
            }
            width: parent.width

            mainTitle: "Proxy"
            content: proxySettings
            yTimer: yAdjustTimer

            Proxy {
                id: proxySettings
                width: parent.width
                visible: parent.isExpanded
            }
        }//ExpandView (proxy)

        ExpandView {
            id: expandUpdateSettings
            anchors {
                top: expandProxySettings.bottom
                left: parent.left
                topMargin: 1
            }
            width: parent.width

            mainTitle: "Auto refresh"
            content: updateSettings
            yTimer: yAdjustTimer

            Updates {
                id: updateSettings
                width: parent.width
                visible: parent.isExpanded
            }
        }//ExpandView (updates)

        ExpandView {
            id: expandEtCetera
            anchors {
                top: expandUpdateSettings.bottom
                left: parent.left
                topMargin: 1
            }
            width: parent.width

            mainTitle: "Et Cetera"
            content: etCetera
            yTimer: yAdjustTimer

            EtCetera {
                id: etCetera
                objectName: "EtCetera"
                width: parent.width
                visible: parent.isExpanded
            }
        }//ExpandView (Et Cetera)
    }//SilicaFlickable
}//Page (SettingsPage)
