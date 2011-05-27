/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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

ListModel {
    ListElement {
        name: "Uv"
        contacts: [
            ListElement {
                type: "Mobile"
                number: "+1 408 111 2222"
            },
            ListElement {
                type: "Work"
                number: "+1 408 333 4444"
            },
            ListElement {
                type: "Home"
                number: "+1 408 555 6666"
            }
        ]
    }
    ListElement {
        name: "Yasho"
        contacts: [
            ListElement {
                type: "Mobile"
                number: "+1 408 777 8888"
            },
            ListElement {
                type: "Work"
                number: "+1 408 999 0000"
            }
        ]
    }
}//ListModel
