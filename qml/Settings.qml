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
    signal sigRefresh
    signal sigRefreshAll
    signal sigHide
    signal sigQuit
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

    property real pixHeight: (height + width) / 30

    contentHeight: expandLoginDetails.height + expandProxySettings.height +
                   expandMqSettings.height + expandPinSettings.height
    contentWidth: width

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
            y: expandLoginDetails.startY

            width: parent.width - 1
            pixHeight: container.pixHeight
            opacity: expandLoginDetails.containedOpacity

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
            y: expandProxySettings.startY

            width: parent.width - 1
            pixHeight: container.pixHeight

            opacity: expandProxySettings.containedOpacity

            onSigProxyChanges: container.sigProxyChanges(bEnable, bUseSystemProxy,
                                                         host, port,
                                                         bRequiresAuth,
                                                         user, pass);
            onSigDone: {
                if (!bSave) {
                    container.sigProxyRefresh();
                }
                expandProxySettings.isExpanded = false;
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
            y: expandMqSettings.startY

            width: parent.width - 1
            pixHeight: container.pixHeight

            opacity: expandMqSettings.containedOpacity

            onSigDone: {
                if (!bSave) {
                    container.sigMosquittoRefresh();
                }
                expandMqSettings.isExpanded = false;
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
            y: expandPinSettings.startY

            width: parent.width - 1
            pixHeight: container.pixHeight

            opacity: expandPinSettings.containedOpacity

            onSigDone: {
                if (!bSave) {
                    container.sigPinRefresh();
                }
                expandPinSettings.isExpanded = false;
            }
            onSigPinSettingChanges: container.sigPinSettingChanges(bEnable, pin)
        }//Pin settings
    }//ExpandView (pin settings)

//    ListView {
//        id: listButtons
//        width: parent.width
//        height: parent.height - (textUsername.height * 2)
//        clip: true

//        model: ListModel {
//            ListElement {
//                text: "Web Page (debug)"
//                newState: "WebPage"
//            }//ListElement (Web Page (debug))
//            ListElement {
//                text: "View Log (debug)"
//                newState: "ViewLog"
//            }//ListElement (View Log (debug))
//            ListElement {
//                text: "Refresh"
//                newState: ""
//            }//ListElement (Refresh)
//            ListElement {
//                text: "About"
//                newState: "About"
//            }//ListElement (About)
//        }

//        delegate: MyButton {
//            mainText: text)
//            width: listButtons.width - 1
//            height: mainColumn.pixHeight / mainColumn.pixDiv
//            mainPixelSize: height * 2 / 3

//            onClicked: {
//                if (newState != "") {
//                    container.state = newState
//                }

//                if (text == "Refresh") {
//                    container.sigRefresh();
//                }
//            }//onClicked

//            onPressHold: {
//                if (text == "Refresh") {
//                    container.sigRefreshAll();
//                }
//            }
//        }//delegate (MyButton)
//    }//ListView

    DbgWebWidget {
        id: myWebWidget
        anchors.fill: parent
        anchors.topMargin: 2
        opacity: 0
        onSigBack: container.state = ''
    }

    LogView {
        id: logView
        anchors.fill: parent
        anchors.topMargin: 2
        opacity: 0
        onSigBack: container.state = ''
    }

    About {
        id: aboutWin
        anchors.fill: parent
        anchors.topMargin: 2
        opacity: 0
        onSigBack: container.state = ''
        onSigLinkActivated: container.sigLinkActivated(strLink)
    }//About

    states: [
        State {
            name: "Mosquitto"
            PropertyChanges { target: mqSettings; opacity: 1 }
            PropertyChanges { target: pinSettings; opacity: 0 }
            PropertyChanges { target: myWebWidget; opacity: 0 }
            PropertyChanges { target: logView; opacity: 0 }
            PropertyChanges { target: aboutWin; opacity: 0 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        },//Mosquitto
        State {
            name: "PinSettings"
            PropertyChanges { target: mqSettings; opacity: 0 }
            PropertyChanges { target: pinSettings; opacity: 1 }
            PropertyChanges { target: myWebWidget; opacity: 0 }
            PropertyChanges { target: logView; opacity: 0 }
            PropertyChanges { target: aboutWin; opacity: 0 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        },//Pin settings
        State {
            name: "WebPage"
            PropertyChanges { target: mqSettings; opacity: 0 }
            PropertyChanges { target: pinSettings; opacity: 0 }
            PropertyChanges { target: myWebWidget; opacity: 1 }
            PropertyChanges { target: logView; opacity: 0 }
            PropertyChanges { target: aboutWin; opacity: 0 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        },//WebPage
        State {
            name: "ViewLog"
            PropertyChanges { target: mqSettings; opacity: 0 }
            PropertyChanges { target: pinSettings; opacity: 0 }
            PropertyChanges { target: myWebWidget; opacity: 0 }
            PropertyChanges { target: logView; opacity: 1 }
            PropertyChanges { target: aboutWin; opacity: 0 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        },//View Log
        State {
            name: "About"
            PropertyChanges { target: mqSettings; opacity: 0 }
            PropertyChanges { target: pinSettings; opacity: 0 }
            PropertyChanges { target: myWebWidget; opacity: 0 }
            PropertyChanges { target: logView; opacity: 0 }
            PropertyChanges { target: aboutWin; opacity: 1 }
            PropertyChanges { target: mainColumn; opacity: 0 }
        }//About
    ]
}// Item (container)
