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
    signal setCiNumber(string id, string number)

    property string ciId: "Replace this ID"
    property alias phonesModel: numbersView.model

    Column {
        anchors.fill: parent
        spacing: 5

        Label {
            id: lblTitle
            font.pixelSize: 30
            text: "Please select a number for the account with id " + container.ciId
        }

        ListView {
            id: numbersView

            width: parent.width
            height: parent.height - lblTitle.height - parent.spacing

            delegate: Label {
                width: numbersView.width
                text: name + "\n(" + number + ")"

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        container.setCiNumber(container.ciId, number);
                        container.done(true);
                    }
                }
            }//delegate
        }//ListView
    }//Column
}//TFA Dialog
