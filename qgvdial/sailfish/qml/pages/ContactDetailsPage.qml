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

Page {
    id: container

    signal done(bool accepted)
    signal setNumberToDial(string number)

    property alias imageSource: contactImage.source
    property alias name: contactName.text
    property alias phonesModel: detailsView.model
    property alias notes: lblNotes.text
    property int modelCount

    PageHeader {
        id: pageHeader
        title: "Contact details"
    }

    SilicaFlickable {
        anchors {
            top: pageHeader.bottom
            bottom: container.bottom
        }
        width: container.width

        contentWidth: mainColumn.width
        contentHeight: mainColumn.height
        clip: true

        Column {
            id: mainColumn

            width: container.width
            height: titleRow.height + lblNotes.height + detailsView.height

            spacing: 5

            Row {
                id: titleRow
                width: parent.width
                height: contactImage.height > contactName.height ?
                            contactImage.height : contactName.height

                Image {
                    id: contactImage
                    fillMode: Image.PreserveAspectFit
                    height: 200
                    width: 200
                }
                Label {
                    id: contactName
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 70
                    smooth: true
                    width: parent.width - contactImage.width - parent.spacing
                    wrapMode: TextEdit.WordWrap
                }
            }

            TextArea {
                id: lblNotes
                anchors.horizontalCenter: parent.horizontalCenter
                smooth: true
                wrapMode: TextEdit.Wrap
                readOnly: true
                width: parent.width - 40
                //platformInverted: true
            }

            Label {
                id: lblInvisible
                visible: false
                opacity: 0
                font.pixelSize: 50
            }

            ListView {
                id: detailsView

                width: parent.width - 40
                height: lblInvisible.height * modelCount
                interactive: false

                delegate: Item {
                    height: lblNumber.height + 10
                    width: detailsView.width

                    Rectangle {
                        id: hitRectNumber
                        anchors.fill: parent
                        color: "orange"
                        opacity: 0
                    }

                    Row {
                        width: parent.width

                        Label {
                            id: lblType
                            text: type
                            font.pixelSize: lblInvisible.font.pixelSize
                        }

                        Label {
                            id: lblNumber
                            text: number
                            width: parent.width - lblType.width - parent.spacing
                            horizontalAlignment: Text.AlignRight
                            font.pixelSize: lblInvisible.font.pixelSize
                        }
                    }//Row: type (work/home/mobile) and number

                    MouseArea {
                        anchors.fill: parent
                        onPressed: { hitRectNumber.opacity = 1.0; }
                        onReleased: { hitRectNumber.opacity = 0.0; }
                        onPressAndHold: {
                            hitRectNumber.opacity = 0.0;
                            container.setNumberToDial(number);
                            container.done(true);
                        }
                    }
                }//delegate
            }//SilicaListView
        }//Column
    }//SilicaFlickable
}//Contact details page
