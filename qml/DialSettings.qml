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

Item {
    id: container

    // Font size
    property real internalPointSize: (8 * g_fontMul)

    height: registeredPhonesView.height + 2

    QGVRadioButton {
        id: delegateRadioButton
        width: parent.width
        pointSize: container.internalPointSize
        text: "mText"
        visible: false
        opacity: 0
    }

    ListView {
        id: registeredPhonesView
        objectName: "RegisteredPhonesView"

        signal sigSelChanged (int index)
        function setSelected(index) {
            var i, val;
            for (i = 0; i < regPhoneModel.count; i++) {
                if (i === index) {
                    val = true;
                } else {
                    val = false;
                }

                regPhoneModel.setProperty(i, "isChecked", val);
            }
        }

        anchors {
            top: parent.top
            left: parent.left
        }
        width: parent.width
        height: ((delegateRadioButton.height + spacing) * regPhoneModel.count) + 5
        interactive: false

        model: ListModel {
            id: regPhoneModel
            objectName: "RegisteredPhonesModel"
        }//ListModel (of the registered phones)

        delegate: QGVRadioButton {
            width: registeredPhonesView.width
            text: entryText
            //checked: (index == container.currentSelection)
            checked: isChecked
            pointSize: container.internalPointSize
            onClicked: registeredPhonesView.sigSelChanged(index);
        }
    }//ListView (of the registered phones)

    // Only for the sake of height calculations
}//Item(container)
