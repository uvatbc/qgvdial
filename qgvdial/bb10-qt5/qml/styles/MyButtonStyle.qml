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

import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Private 1.0

Style {
    property Component panel: Item {
        implicitWidth: Math.max(300, buttonText.width + 50);
        implicitHeight: buttonImg.sourceSize.height
        BorderImage {
            id: buttonImg
            width: parent.width
            border { left: buttonImg.sourceSize.width/2-1; right: buttonImg.sourceSize.width/2-1; }
            horizontalTileMode: BorderImage.Stretch
            source: control.enabled ? control.pressed ? "assets/button/colt/dark/core_button_pressed.png"
                                                               :"assets/button/colt/dark/core_button_inactive.png"
                                      : "assets/button/colt/dark/core_button_disabled.png"
        }
        Text {
            anchors.centerIn: parent
            id: buttonText
            text: control.text
            font.pixelSize: 35
            color: __syspal.text

        }
    }
}
