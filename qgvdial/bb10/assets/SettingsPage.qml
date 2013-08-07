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

NavigationPane {
    id: container
    
    Page {
        content: ListView {
            function callMe() {
                console.debug("Do mee");
                container.push(loginDetails);
            }
            
            dataModel: ArrayDataModel {
                id: settingsModel
            }
            
            onCreationCompleted: {
                settingsModel.append("Login details");
            }
            
            listItemComponents: [
                ListItemComponent {
                    Label {
                        text: ListItem.data
                        onTouch: {
                            if (ListItem.indexPath == 0) {
                                ListItem.view.callMe();
                            }
                        }
                    }
                }
            ]
        }
    }
    
    attachedObjects: [
        Page {
            id: loginDetails
            Container {
                Label {
                    text: "Username"
                }
                Label {
                    text: "Password"
                }
            }
        }
    ]
}//NavigationPane
