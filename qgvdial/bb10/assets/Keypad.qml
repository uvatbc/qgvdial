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

import bb.cascades 1.0

Container {
    id: container
    
    signal keyPress(string text)
    signal call
    signal text
    signal del

    layout: StackLayout { orientation: LayoutOrientation.TopToBottom }
    
    Container {
        layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
        layoutProperties: StackLayoutProperties { spaceQuota: 1.0 }

        KeyButton {
            mainText: "1"
            onClicked: container.keyPress(mainText);
        }
        KeyButton {
            mainText: "2"
            subText: "ABC"
            onClicked: container.keyPress(mainText);
        }
        KeyButton {
            mainText: "3"
            subText: "DEF"
            onClicked: container.keyPress(mainText);
        }
    }//Row 1

    Container {
        layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
        layoutProperties: StackLayoutProperties { spaceQuota: 1.0 }

        KeyButton {
            mainText: "4"
            subText: "GHI"
            onClicked: container.keyPress(mainText);
        }
        KeyButton {
            mainText: "5"
            subText: "JKL"
            onClicked: container.keyPress(mainText);
        }
        KeyButton {
            mainText: "6"
            subText: "MNO"
            onClicked: container.keyPress(mainText);
        }
    }//Row 2

    Container {
        layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
        layoutProperties: StackLayoutProperties { spaceQuota: 1.0 }

        KeyButton {
            mainText: "7"
            subText: "PQRS"
            onClicked: container.keyPress(mainText);
        }
        KeyButton {
            mainText: "8"
            subText: "TUV"
            onClicked: container.keyPress(mainText);
        }
        KeyButton {
            mainText: "9"
            subText: "WXYZ"
            onClicked: container.keyPress(mainText);
        }
    }//Row 3

    Container {
        layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
        layoutProperties: StackLayoutProperties { spaceQuota: 1.0 }

        KeyButton {
            mainText: "*"
            onClicked: container.keyPress(mainText);
        }
        KeyButton {
            mainText: "0"
            subText: "+"
            onClicked: container.keyPress(mainText);
            onPressAndHold: container.keyPress(subText);
        }
        KeyButton {
            mainText: "#"
            onClicked: container.keyPress(mainText);
        }
    }//Row 4

    Container {
        layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
        layoutProperties: StackLayoutProperties { spaceQuota: 1.0 }

        Button {
            text: "Call"
            onClicked: container.call();
        }
        Button {
            text: "Text"
            onClicked: container.text();
        }
        Button {
            text: "\u232B"
            onClicked: container.del();
        }
    }//Row 5
}//Container
