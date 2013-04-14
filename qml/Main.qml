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

import Qt 4.7
import "meego"
import "generic"
import "s3"

Rectangle {
    id: main
    objectName: "MainPage"

    width: 250
    height: 400

    color: "black"

    // Signals from dialpad, contacts and inbox
    signal sigCall(string number)
    signal sigText(string numbers, string smsText)
    // Signals from the Settings page
    signal sigHide
    signal sigQuit

    // From the contacts page
    signal sigRefreshContacts
    signal sigRefreshAllContacts

    // Contacts search query changes
    signal sigSearchContacts(string query)

    // Hapetic feedback required
    signal sigHaptic

    function doRotate(rot) {
        console.debug("Rotate by " + rot);
        main.rotation = rot;
        console.debug("Rotated by " + main.rotation);
    }

    function showSmsView() {
        mainFlipView.flipped = true;
    }

    onSigCall: console.debug("QML: Call " + number)
    onSigText: console.debug("QML: Text " + numbers)

    onSigSearchContacts: console.debug("QML: Search for contacts: " + query)

    onSigHide: console.debug("QML: Dismiss requested");
    onSigQuit: console.debug("QML: Quit requested");

    property bool bShowSettings: g_bShowLoginSettings
    onBShowSettingsChanged: {
        if (bShowSettings) {
            console.debug("Settings page shown");
            main.state = "Settings";
        } else {
            console.debug("Settings off");
            main.state = '';
        }
    }

    Flipable {
        id: mainFlipView

        property bool flipped: false

        opacity: (1 - loginPage.opacity)

        anchors {
            top:    parent.top
            left:   parent.left
            right:  parent.right
            bottom: barStatus.top
        }

        front: Item {
            id: mainColumn

            anchors.fill: parent
            property int centralHeight: mainColumn.height - barStatus.height
            property int centralWidth: mainColumn.width

            VisualItemModel {
                id: tabsModel
                Tab {
                    icon: "dialpad.svg"

                    MainView {
                        id: dialPad
                        anchors.fill: parent

                        onSigCall: main.sigCall (number)
                        onSigText: {
                            mainFlipView.flipped = true;
                            smsView.addSmsDestination(number, number);
                        }

                        onSigHaptic: main.sigHaptic();
                    }
                }//Tab (Dialpad)
                Tab {
                    icon: "people.svg"

                    ContactsList {
                        id: contactsList

                        anchors.fill: parent

                        onSigCall: main.sigCall (number)
                        onSigSearchContacts: main.sigSearchContacts(query)
                        onSigText: {
                            mainFlipView.flipped = true;
                            smsView.addSmsDestination(name, number);
                        }

                        onSigRefreshContacts: main.sigRefreshContacts();
                        onSigRefreshAllContacts: main.sigRefreshAllContacts();
                    }
                }//Tab (Contacts)
                Tab {
                    icon: "history.svg"

                    InboxList {
                        id: inboxList
                        objectName: "InboxPage"

                        anchors.fill: parent

                        onSigCall: main.sigCall (number)
                        onSigText: {
                            mainFlipView.flipped = true;
                            smsView.addSmsDestination(name, number);
                        }
                    }
                }//Tab (Inbox)
                Tab {
                    icon: "settings.svg"
                    color: "black"

                    Settings {
                        id: settingsView
                        anchors.fill: parent
                    }
                }//Tab (Settings)
            }//VisualDataModel (contains the tabs)

            TabbedUI {
                id: tabbedUI
                objectName: "TabbedUI"

                tabsHeight: (main.height + main.width) / 20
                tabIndex: 3
                tabsModel: tabsModel
                anchors {
                    top: parent.top
                    topMargin: 1
                    bottomMargin: 1
                }
                width: mainColumn.centralWidth - 1
                height: mainColumn.centralHeight - 1

                onSigHide: main.sigHide();
                onSigClose: main.sigQuit();
            }
        }//Item: Main column that has all the co-existent views

        back: SmsView {
            id: smsView
            anchors.fill: parent

            onSigBack: mainFlipView.flipped = false;
            onSigText: {
                main.sigText(strNumbers, strText);
                mainFlipView.flipped = false;
            }
        }

        transform: Rotation {
            id: flipRotation
            origin.x: mainFlipView.width/2
            origin.y: mainFlipView.height/2
            axis.x: 0; axis.y: 1; axis.z: 0     // set axis.y to 1 to rotate around y-axis
            angle: 0    // the default angle
        }

        states: State {
            name: "back"
            PropertyChanges { target: flipRotation; angle: 180 }
            when: mainFlipView.flipped
        }

        transitions: Transition {
            NumberAnimation { target: flipRotation; property: "angle"; duration: 500 }
        }
    }//Flipable

    LoginPage {
        id: loginPage
        objectName: "LoginPage"
        anchors.fill: parent
        opacity: 1

        onSigHide: opacity = 0;
    }//LoginPage

    MsgBox {
        id: msgBox
        objectName: "MsgBox"

        opacity: 0
        msgText: "No message"

        anchors.fill: parent
    }//MsgBox

    Rectangle {
        id: barStatus
        width: parent.width
        height: (parent.height + parent.width) / 30
        anchors.bottom: parent.bottom

        color: "black"

        QGVLabel {
            text: g_strStatus
            fontPointMultiplier: 7.0 / 8.0
            anchors {
                verticalCenter: parent.verticalCenter
                horizontalCenter: parent.horizontalCenter
            }
        }//QGVLabel (status text)
    }//Rectangle (status bar)
}//Rectangle (main)
