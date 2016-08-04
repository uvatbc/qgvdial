/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016  Yuvraaj Kelkar

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

    signal done(bool accepted)
    signal setNumberToDial(string number)

    property alias imageSource: contactImage.source
    property alias name: contactName.text
    property alias phonesModel: detailsView.model
    property alias notes: lblNotes.text
    property int modelCount

    Flickable {
        width: container.width
        height: container.height

        contentWidth: mainColumn.width
        contentHeight: mainColumn.height

        Column {
            id: mainColumn

            width: parent.width
            height: titleRow.height + lblNotes.height + detailsView.height

            spacing: 5

            Row {
                id: titleRow
                width: container.width
                height: contactImage.height > contactName.height ? contactImage.height : contactName.height

                Image {
                    id: contactImage
                    fillMode: Image.PreserveAspectFit
                    height: 100
                    width: height
                }
                Label {
                    id: contactName
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 40
                    smooth: true
                    width: parent.width - contactImage.width - parent.spacing
                }
            }//row: contact image and name

            TextArea {
                id: lblNotes
                anchors.horizontalCenter: parent.horizontalCenter
                smooth: true
                wrapMode: TextEdit.Wrap
                readOnly: true
                width: parent.width - 40
            }

            Label {
                id: lblInvisible
                visible: false
                opacity: 0
                font.pixelSize: 25
            }

            ListView {
                id: detailsView

                width: parent.width - 40
                height: lblInvisible.height * modelCount

                anchors.horizontalCenter: parent.horizontalCenter
                interactive: false

                delegate: Item {
                    height: lblNumber.height + 4
                    width: container.width - 40

                    Rectangle {
                        id: hitRectNumber
                        anchors.fill: parent
                        color: "orange"
                        opacity: 0
                    }

                    Row {
                        width: parent.width
                        anchors.horizontalCenter: parent.horizontalCenter

                        Label {
                            id: lblType
                            text: type
                        }

                        Label {
                            id: lblNumber
                            text: number
                            width: parent.width - lblType.width - parent.spacing
                            horizontalAlignment: Text.AlignRight
                            font.pixelSize: lblInvisible.font.pixelSize
                            smooth: true
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
            }//ListView
        }//Column
    }//Flickable
}//Contact details Page
