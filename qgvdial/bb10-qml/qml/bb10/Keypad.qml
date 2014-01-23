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

import QtQuick 1.1
import com.nokia.symbian 1.1

Rectangle {
    id: container
    color: "black"

    signal btnClick(string strText)
    //signal btnDelClick
    signal sigHaptic

    onBtnClick: container.sigHaptic();
    //onBtnDelClick: container.sigHaptic();

    Grid {
        id: layoutGrid
        anchors.fill: parent
        rows: 4; columns: 3
        spacing: 1

        DigitButton { mainText: "1"; subText: ""
            onClicked: container.btnClick(strText);
        }
        DigitButton { mainText: "2"; subText: "ABC"
            onClicked: container.btnClick(strText);
        }
        DigitButton { mainText: "3"; subText: "DEF"
            onClicked: container.btnClick(strText);
        }
        DigitButton { mainText: "4"; subText: "GHI"
            onClicked: container.btnClick(strText);
        }
        DigitButton { mainText: "5"; subText: "JKL"
            onClicked: container.btnClick(strText);
        }
        DigitButton { mainText: "6"; subText: "MNO"
            onClicked: container.btnClick(strText);
        }
        DigitButton { mainText: "7"; subText: "PQRS"
            onClicked: container.btnClick(strText);
        }
        DigitButton { mainText: "8"; subText: "TUV"
            onClicked: container.btnClick(strText);
        }
        DigitButton { mainText: "9"; subText: "WXYZ"
            onClicked: container.btnClick(strText);
        }
        DigitButton { mainText: "*+"; subText: ""
            onClicked: container.btnClick(strText);
            //onDelClicked: container.btnDelClick();
        }
        DigitButton { mainText: "0"; subText: ""
            onClicked: container.btnClick(strText);
        }
        DigitButton { mainText: "#"; subText: ""
            onClicked: container.btnClick(strText);
        }
    }// Grid
}// Rectangle
