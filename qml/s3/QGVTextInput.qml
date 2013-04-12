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
import com.nokia.symbian 1.0

TextField {
    id:  container

    signal sigTextChanged(string strText)
    signal sigEnter

    text: "You should have changed this text"
    property int pointSize: (10 * g_fontMul)
    property real fontPointMultiplier: 1.0

    inputMethodHints: Qt.ImhNoAutoUppercase + Qt.ImhNoPredictiveText

    function closeSoftwareInputPanel() {
        // Symbian doesn't have a cloe software input panel function...
        console.debug("User requested closeSoftwareInputPanel");
    }

    function doAccepted() {
        container.closeSoftwareInputPanel();
        container.sigEnter();
    }

    Keys.onReturnPressed: {
        container.doAccepted();
        event.accepted = true;
    }

    onTextChanged: {
        container.sigTextChanged(text);
    }
}//Rectangle
