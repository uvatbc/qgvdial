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
import com.kdab.components 1.0

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
                        ImageView {
                            image: {
                                switch (ListItemData.type) {
                                    case 1: // GVIE_Placed
                                        return "assets://icons/in_Placed.png";
                                    case 2: // GVIE_Received
                                        return "assets://icons/in_Received.png";
                                    case 3: // GVIE_Missed
                                        return "assets://icons/in_Missed.png";
                                    case 4: // GVIE_Voicemail
                                        return "assets://icons/in_Voicemail.png";
                                    case 5: // GVIE_TextMessage
                                        return "assets://icons/in_Sms.png";
                                }
                            }
                        }
                        Label {
                            text: ListItemData.name
                        }
                    }
                }
            ]
        }
    }//Container
}//Page
