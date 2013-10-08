/*
 * qgvdial is a cross platform Google Voice Dialer
 * Copyright (C) 2009-2013  Yuvraaj Kelkar
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Contact: yuvraaj@gmail.com
 */

import bb.cascades 1.0
import com.kdab.components 1.0

Page {
    id: container
    objectName: "ContactsPage"
    
    
    attachedObjects: [
        AbstractItemModel {
            id: contactsModel

            sourceModel: g_contactsModel
        }
    ]

    Container {
        TextField {
            hintText: "Search"
        }

        ListView {
            objectName: "ContactsList"
            signal clicked(string id);

            dataModel: contactsModel

            listItemComponents: [
                ListItemComponent {
                    Container {
                        id:  listItem

                        layout: StackLayout {
                            orientation: LayoutOrientation.LeftToRight
                        }
                        
                        ImageView {
                            imageSource: {
                                if ((ListItemData.imagePath != null) && (ListItemData.imagePath.length != 0)) {
                                    return ListItemData.imagePath;  
                                } else {
                                    return "asset:///icons/unknown_contact.png";
                                }
                            }
                            
                            scalingMethod: ScalingMethod.AspectFit
                            horizontalAlignment: HorizontalAlignment.Center
                            verticalAlignment: VerticalAlignment.Center
                            
                            preferredHeight: 80
                            preferredWidth: 80
                        }
                        
                        Label {
                            text: ListItemData.name
                            textStyle { base: tsdxlarge.style }
                            verticalAlignment: VerticalAlignment.Center
                        }

                        gestureHandlers: [
                            TapHandler {
                                onTapped: {
                                    listItem.ListItem.view.clicked(ListItemData.id);
                                }                                
                            }
                        ]
                                                 
                        attachedObjects: [
                            TextStyleDefinition {
                                id: tsdxlarge
                                base: SystemDefaults.TextStyles.BodyText
                                fontSize: FontSize.XLarge
                            }
                        ]//attachedObjects
                    }
                }
            ]//listItemComponents
        }//ListView
    }//Container
}//Page
