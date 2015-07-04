/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2015  Yuvraaj Kelkar

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

import QtQuick 2.2
import QtQuick.Controls 1.1

Rectangle {
    id: container

    signal done(bool accepted)
    signal setNumberToDial(string number)
    signal replySms(string iId)

    signal play
    signal pause
    signal stop

    // These fields are filled up by the JS function showInboxDetails
    property alias imageSource: contactImage.source
    property alias name:        contactName.text
    property alias number:      contactNumber.text
    property alias phType:      numberType.text
    property alias note:        lblNote.text
    property alias smsText:     lblSmsText.text
    property bool isVmail
    property string cId
    property string iId

    // These fields are updated while playing the voicemail
    property bool showPlayBtn: true
    property bool fetchingEmail: true
    property real vmailDuration
    property real vmailPosition

    color: "black"
    visible: false

    Flickable {
        anchors {
            top: parent.top
            left: parent.left
            bottom: btnRow.top
        }
        width: parent.width

        clip: true

        contentWidth: width
        contentHeight: mainColumn.height

        Column {
            id: mainColumn

            spacing: 20
            width: parent.width

            Item {
                width: parent.width
                height: 200

                Rectangle {
                    id: hitRectContact
                    anchors.fill: parent
                    color: "orange"
                    opacity: 0
                }

                Row {
                    anchors.fill: parent

                    Image {
                        id: contactImage
                        fillMode: Image.PreserveAspectFit
                        height: parent.height
                        width: height
                    }
                    Label {
                        id: contactName
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: 70
                        smooth: true
                        width: parent.width - contactImage.width - parent.spacing
                    }
                }//Row: Contact image and name

                MouseArea {
                    anchors.fill: parent
                    onPressed: { hitRectContact.opacity = 1.0; }
                    onReleased: { hitRectContact.opacity = 0.0; }
                    onPressAndHold: {
                        hitRectContact.opacity = 0.0;
                        if (container.cId.length != 0) {
                            g_contacts.getContactInfoAndModel(container.cId);
                        }
                    }
                }
            }//Item: Contact image and name

            Item {
                width: parent.width
                height: 60

                Rectangle {
                    id: hitRectNumber
                    anchors.fill: parent
                    color: "orange"
                    opacity: 0
                }

                Row {
                    anchors.fill: parent
                    spacing: 10

                    Label {
                        id: numberType

                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: 50
                    }

                    Label {
                        id: contactNumber

                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - numberType.width - parent.spacing
                        font.pixelSize: 50
                        horizontalAlignment: Text.AlignRight
                    }
                }//Row: type (mobile/work/home) and actual number

                MouseArea {
                    anchors.fill: parent
                    onPressed: { hitRectNumber.opacity = 1.0; }
                    onReleased: { hitRectNumber.opacity = 0.0; }
                    onPressAndHold: {
                        hitRectNumber.opacity = 0.0;
                        container.setNumberToDial(container.number);
                        container.done(true);
                    }
                }
            }//Item: type (mobile/work/home) and actual number

            Label {
                id: lblNote
                width: parent.width
                visible: text.length == 0 ? false : true
                font.pixelSize: 45
                wrapMode: Text.WordWrap
            }//Label: notes

            Label {
                id: lblSmsText
                width: parent.width
                visible: text.length == 0 ? false : true
                font.pixelSize: 45
                wrapMode: Text.WordWrap
            }//Label: SMS text

            ProgressBar {
                id: vmailProgress
                minimumValue: 0
                maximumValue: vmailDuration
                value: vmailPosition
                width: parent.width * 7/10
                visible: (container.isVmail && !container.fetchingEmail)
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Row {
                spacing: 25
                visible: vmailProgress.visible
                anchors.horizontalCenter: parent.horizontalCenter

                Image {
                    source: container.showPlayBtn ? "qrc:/button_black_play.png" : "qrc:/button_black_pause.png"
                    height: 100
                    width: height
                    smooth: true

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (container.showPlayBtn) {
                                container.play();
                            } else {
                                container.pause();
                            }
                        }
                    }
                }
                Image {
                    source: "qrc:/button_black_stop.png"
                    height: 100
                    width: height
                    smooth: true

                    MouseArea {
                        anchors.fill: parent
                        onClicked: { container.stop(); }
                    }
                }
            }

            Label {
                text: "Fetching voicemail"
                visible: (container.isVmail && container.fetchingEmail)
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }//Column
    }//Flickable

    Row {
        id: btnRow
        anchors {
            bottom: parent.bottom
            bottomMargin: 5
            horizontalCenter: parent.horizontalCenter
        }
        spacing: 5

        width: (btnReply.visible ? parent.width * 8 / 10
                                 : parent.width * 4 / 10)

        Button {
            id: btnReply
            text: "Reply"
            visible: (smsText.length != 0)
            width: ((container.width - parent.spacing) / 2) * 8 / 10

            onClicked: {
                container.replySms(container.iId);
            }
        }
        Button {
            text: "Delete"

            width: (btnReply.visible ? (((container.width - parent.spacing) / 2) * 8 / 10)
                                     : parent.width)

            onClicked: {
                g_inbox.deleteEntry(container.iId);
                container.done(true);
            }
        }
    }//ButtonRow
}//Page
