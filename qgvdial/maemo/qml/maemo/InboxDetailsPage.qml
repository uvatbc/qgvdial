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

Rectangle {
    id: container

    signal done(bool accepted)
    signal setNumberToDial(string number)
    signal sigOpenContact(string cId)
    signal deleteEntry(string iId)
    signal replySms(string iId)

    signal play
    signal pause
    signal stop

    // These fields are filled up by the JS function showInboxDetails
    property alias imageSource: contactImage.source
    property alias name:        contactName.text
    property alias number:      contactNumber.text
    property alias phType:      numberType.text
    property string note
    property string smsText
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
    opacity: visible ? 1 : 0
    Behavior on opacity { PropertyAnimation { duration: 250 } }

    Flickable {
        anchors {
            top: parent.top
            left: parent.left
            bottom: btnRow.top
        }
        width: parent.width

        contentWidth: width
        contentHeight: mainColumn.height
        clip: true

        Column {
            id: mainColumn

            spacing: 10
            width: parent.width

            Item {
                width: parent.width
                height: 100

                Row {
                    anchors.fill: parent

                    Image {
                        id: contactImage
                        fillMode: Image.PreserveAspectFit
                        height: parent.height
                        width: height
                    }
                    TextOneLine {
                        id: contactName
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: 40
                        width: parent.width - contactImage.width - parent.spacing
                        clip: true
                        readOnly: true
                        enableBorder: false
                        color: "transparent"
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onPressAndHold: {
                        container.done(true);
                        if (container.cId.length != 0) {
                            container.sigOpenContact(container.cId);
                        }
                    }
                }
            }//contact image and title

            Item {
                width: parent.width
                height: numberType.height

                TextOneLine {
                    id: numberType
                    font.pixelSize: 30
                    readOnly: true
                    enableBorder: false
                    color: "transparent"
                    width: 170
                    horizontalAlignment: Text.AlignLeft

                    anchors {
                        left: parent.left
                        verticalCenter: parent.verticalCenter
                    }
                }//type: mobile/work/home

                TextOneLine {
                    id: contactNumber
                    width: parent.width - numberType.width - 2
                    font.pixelSize: 30
                    horizontalAlignment: Text.AlignRight
                    readOnly: true
                    enableBorder: false
                    color: "transparent"

                    anchors {
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                    }
                }//number

                MouseArea {
                    anchors.fill: parent
                    onPressAndHold: {
                        container.setNumberToDial(container.number);
                        container.done(true);
                    }
                }
            }//Item: type (mobile/work/home) and actual number

            TextMultiLine {
                id: txtNotes
                width: parent.width
                enableBorder: false
                readOnly: true
                wrapMode: Text.WordWrap
                textFormat: Text.RichText

                text: container.note
                visible: (container.note.length != 0)
            }//notes

            TextMultiLine {
                id: txtSms
                width: parent.width
                enableBorder: true
                readOnly: true
                wrapMode: Text.WordWrap
                textFormat: Text.RichText

                text: container.smsText
                visible: (container.smsText.length != 0)

                onHeightChanged: console.debug("h = " + height)
            }//SMS text

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
                    height: 70
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
                    height: 70
                    width: height
                    smooth: true

                    MouseArea {
                        anchors.fill: parent
                        onClicked: { container.stop(); }
                    }
                }
            }

            Text {
                text: "Fetching email"
                visible: (container.isVmail && container.fetchingEmail)
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }//Column
    }//Flickable

    Row {
        id: btnRow
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
        width: parent.width * 8 / 10

        Button {
            text: "Reply"
            visible: (smsText.length != 0)
            width: ((container.width - parent.spacing) / 2) * 8 / 10

            onClicked: {
                container.replySms(container.iId);
            }
        }
        Button {
            text: "Delete"
            width: ((container.width - parent.spacing) / 2) * 8 / 10

            onClicked: {
                container.deleteEntry(container.iId);
                container.done(true);
            }
        }
    }//ButtonRow
}//Item
