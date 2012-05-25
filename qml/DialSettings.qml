/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

Item {
    id: container

    height: mainColumn.height + 2

    Column {
        id: mainColumn

        anchors {
            top: parent.top
            left: parent.left
        }
        width: parent.width
        height: childrenRect.height

        ListView {
            id: registeredPhonesView
            objectName: "RegisteredPhonesView"

            signal sigSelChanged (int index)
            function setSelected(index) {
                var i;
                for (i = 0; i < regPhoneModel.count; i++) {
                    regPhoneModel.setProperty (i, "isChecked", (i === index));
                }
            }

            width: parent.width
            height: ((delegateRadioButton.height + spacing) * regPhoneModel.count) + 5
            interactive: false

            model: ListModel {
                id: regPhoneModel
                objectName: "RegisteredPhonesModel"
            }//ListModel (of the registered phones)

            delegate: RadioButton {
                width: registeredPhonesView.width
                text: entryText
                check: isChecked
                autoChange: false
                onClicked: {
                    registeredPhonesView.sigSelChanged(index);
                }
            }
        }//ListView (of the registered phones)
    }// Main

    // Only for the sake of height calculations
    RadioButton {
        id: delegateRadioButton
        width: parent.width
        text: "entryText"
        visible: false
        opacity: 0
    }
}//Item(container)
