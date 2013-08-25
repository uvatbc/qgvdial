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

Page {
    attachedObjects: [
        AbstractItemModel {
            id: inboxModel
            
            sourceModel: g_inboxModel
        }
    ]
    
    Container {
        ListView {
            dataModel: inboxModel
            
            listItemComponents: [
                ListItemComponent {
                    Container {
                        layout: StackLayout {
                            orientation: LayoutOrientation.LeftToRight
                        }
  /*                      
                        ImageView {
                            image: {
                                if ((ListItemData.imagePath != null) && ListItemData.imagePath.length != 0) {
                                    return ListItemData.imagePath;  
                                } else {
                                    return "assets:///icons/unknown_contact.png";
                                }
                            }
                        }
*/
                        Label {
                            text: ListItemData.name
                        }
                    }
                }
            ]
        }
    }//Container
}//Page
