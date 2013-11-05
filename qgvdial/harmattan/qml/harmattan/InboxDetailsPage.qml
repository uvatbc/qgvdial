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

    signal done(bool accepted)
    signal setNumberToDial(string number)

    property alias imageSource: contactImage.source
    property alias name: contactName.text
    property alias number: contactNumber.text
    property alias phType: numberType.text

    Column {
        anchors.fill: parent
        spacing: 20

        Row {
            id: titleRow
            width: parent.width
            height: contactImage.height

            Image {
                id: contactImage
                fillMode: Image.PreserveAspectFit
                height: 100
                width: 100
            }
            Label {
                id: contactName
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 40
                smooth: true
                width: parent.width - contactImage.width - parent.spacing
            }
        }

        Row {
            width: parent.width
            spacing: 10

            Label {
                id: numberType
                font.pixelSize: 30

                MouseArea {
                    anchors.fill: parent
                    onPressAndHold: {
                        container.setNumberToDial(container.number);
                        container.done(true);
                    }
                }
            }

            Label {
                id: contactNumber
                width: parent.width - numberType.width - parent.spacing
                font.pixelSize: 25
                smooth: true
                horizontalAlignment: Text.AlignRight

                MouseArea {
                    anchors.fill: parent
                    onPressAndHold: {
                        container.setNumberToDial(container.number);
                        container.done(true);
                    }
                }
            }
        }
    }//Column
}//Page
