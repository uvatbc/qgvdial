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
    signal setCiNumber(string id, string number)

    property string ciId: "Replace this ID"
    property alias phonesModel: numbersView.model

    PageHeader {
        id: title
        title: "Please select a number for:\n" + container.ciId
    }

    SilicaListView {
        id: numbersView

        anchors {
            top: title.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        delegate: Label {
            width: numbersView.width
            text: name + "\n(" + number + ")"
            font.pixelSize: 25

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    container.setCiNumber(container.ciId, number);
                    container.done(true);
                }
            }
        }//delegate
    }//SilicaListView
}//TFA Dialog
