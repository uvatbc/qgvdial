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
import com.nokia.symbian 1.1

Page {
    id: container
    tools: commonTools

    signal done(bool accepted)
    signal setNumberToDial(string number)
    signal sigShowContact(string cId)
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

    Flickable {
        anchors {
            top: parent.top
            left: parent.left
            bottom: btnRow.top
        }
        width: parent.width

        contentWidth: width
        contentHeight: mainColumn.height

        Column {
            id: mainColumn

            spacing: 20
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
                    Label {
                        id: contactName
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: 40
                        smooth: true
                        width: parent.width - contactImage.width - parent.spacing
                    }
                }//Row: Contact image and name

                MouseArea {
                    anchors.fill: parent
                    onPressAndHold: {
                        if (container.cId.length != 0) {
                            container.sigShowContact(container.cId);
                        }
                    }
                }
            }//Item: Contact image and name

            Item {
                width: parent.width
                height: 40

                Row {
                    anchors.fill: parent
                    spacing: 10

                    Label {
                        id: numberType

                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: 30
                    }

                    Label {
                        id: contactNumber

                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - numberType.width - parent.spacing
                        font.pixelSize: 25
                        smooth: true
                        horizontalAlignment: Text.AlignRight
                    }
                }//Row: type (mobile/work/home) and actual number

                MouseArea {
                    anchors.fill: parent
                    onPressAndHold: {
                        container.setNumberToDial(container.number);
                        container.done(true);
                    }
                }
            }//Item: type (mobile/work/home) and actual number

            Label {
                id: lblNote
                width: parent.width
                visible: text.length == 0 ? false : true
            }//Label: notes

            Label {
                id: lblSmsText
                width: parent.width
                visible: text.length == 0 ? false : true
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

            Label {
                text: "Fetching email"
                visible: (container.isVmail && container.fetchingEmail)
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }//Column
    }//Flickable

    ButtonRow {
        id: btnRow
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
        width: parent.width * 8 / 10
        exclusive: false

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
}//Page
