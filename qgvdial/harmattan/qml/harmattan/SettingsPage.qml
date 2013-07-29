/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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

Page {
    id: container
    tools: commonTools

    objectName: "SettingsPage"

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

    property real toolbarHeight: 50

    Timer {
        id: yAdjustTimer

        property real setY: 0.0
        onSetYChanged: {
            yAdjustTimer.stop();
            yAdjustTimer.start();
        }

        interval: 300; repeat: false; running: false
        onTriggered: container.contentY = setY;
    }//Timer for delayed setting of contentY so that when you click on an
    // ExpandView, it gets into focus. Expecially important for the logs

    Flickable {
        // Cannot use childrenRect.height because it causes a binding loop
        /*
        contentHeight: expandLoginDetails.height + expandProxySettings.height +
                       expandDialSettings.height + expandRefreshSettings.height +
                       expandPinSettings.height + expandLogView.height +
                       expandPhoneIntegration.height + expandAbout.height
        */
        contentHeight: expandLoginDetails.height
        contentWidth: width
        clip: true

        anchors {
            top: parent.top
            topMargin: 40
        }
        width: parent.width
        height: parent.height - container.toolbarHeight

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
            }
        }//ExpandView (login/logout)

/*
        ExpandView {
            id: expandDialSettings
            anchors {
                top: expandLoginDetails.bottom
                left: parent.left
            }

            width: parent.width
            contentHeight: dialSettings.height;

            onClicked: if (isExpanded) yAdjustTimer.setY = y;

            mainTitle: "Dial settings"

            DialSettings {
                id: dialSettings
                y: parent.startY

                width: parent.width - 1
                opacity: parent.containedOpacity
            }
        }//ExpandView (dial settings)

        ExpandView {
            id: expandProxySettings
            anchors {
                top: expandDialSettings.bottom
                left: parent.left
            }

            width: parent.width
            contentHeight: proxySettings.height;

            onClicked: if (isExpanded) yAdjustTimer.setY = y;

            mainTitle: "Proxy"

            Proxy {
                id: proxySettings
                y: parent.startY

                width: parent.width - 1
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
            id: expandRefreshSettings
            anchors {
                top: expandProxySettings.bottom
                left: parent.left
            }

            width: parent.width
            contentHeight: refreshSettings.height;

            onClicked: if (isExpanded) yAdjustTimer.setY = y;

            mainTitle: "Auto refresh"

            RefreshSettings {
                id: refreshSettings
                objectName: "RefreshSettingsPage"

                y: parent.startY
                width: parent.width - 1
                opacity: parent.containedOpacity

                onSigDone: parent.isExpanded = false;
            }//RefreshSettings
        }//ExpandView (refresh)

        ExpandView {
            id: expandPinSettings
            anchors {
                top: expandRefreshSettings.bottom
                left: parent.left
            }

            width: parent.width
            contentHeight: pinSettings.height;

            onClicked: if (isExpanded) yAdjustTimer.setY = y;

            mainTitle: "Pin"

            PinSetting {
                id: pinSettings
                y: parent.startY

                width: parent.width - 1
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

            onClicked: if (isExpanded) yAdjustTimer.setY = y;

            mainTitle: "Logs"

            LogView {
                id: logView

                width: parent.width - 1
                height: container.height - parent.textHeight
                y: parent.startY

                opacity: parent.containedOpacity
                onSigBack: parent.isExpanded = false;
                onSigSendLogs: container.sigSendLogs();
            }
        }//ExpandView (log view)

        ExpandView {
            id: expandPhoneIntegration
            anchors {
                top: expandLogView.bottom
                left: parent.left
            }

            width: parent.width
            contentHeight: phoneIntegrationView.height;

            onClicked: if (isExpanded) yAdjustTimer.setY = y;

            mainTitle: "Phone integration"

            PhoneIntegrationView {
                id: phoneIntegrationView

                width: parent.width
                opacity: parent.containedOpacity
                y: parent.startY
            }
        }//ExpandView (phone integration)

        ExpandView {
            id: expandAbout
            anchors {
                top: expandPhoneIntegration.bottom
                left: parent.left
            }

            width: parent.width
            contentHeight: aboutWin.height;

            onClicked: if (isExpanded) yAdjustTimer.setY = y;

            mainTitle: "About"

            About {
                id: aboutWin

                width: parent.width - 1
                y: parent.startY
                opacity: parent.containedOpacity

                onSigLinkActivated: container.sigLinkActivated(strLink)
            }//About
        }//ExpandView (About)
        */
    }//Flickable
}// Item (container)
