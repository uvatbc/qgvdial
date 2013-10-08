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
    id: container
    signal setNumberToDial(string number)

    attachedObjects: [
        AbstractItemModel {
            id: inboxModel
            
            sourceModel: g_inboxModel
        }
    ]
    
    Container {
        ListView {
            objectName: "InboxList"
            signal clicked(string id);
            
            function setNumberToDial(string number) {
                container.setNumberToDial(number);
            }

            dataModel: inboxModel
            
            listItemComponents: [
                ListItemComponent {
                    Container {
                        id:  listItem
                        layout: DockLayout {}
                        
                        Container {
                            verticalAlignment: VerticalAlignment.Center
                            horizontalAlignment: HorizontalAlignment.Left
                            
                            layout: StackLayout {
                                orientation: LayoutOrientation.LeftToRight
                            }
                            
                            ImageView {
                                imageSource: {
                                    switch (ListItemData.type) {
                                        case "Placed":
                                            return "asset:///icons/in_Placed.png";
                                        case "Received":
                                            return "asset:///icons/in_Received.png";
                                        case "Missed":
                                            return "asset:///icons/in_Missed.png";
                                        case "Voicemail":
                                            return "asset:///icons/in_Voicemail.png";
                                        case "SMS":
                                            return "asset:///icons/in_Sms.png";
                                        default:
                                            console.debug("Invalid type value: " + ListItemData.type);                                            
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
                            }
                        }
                        
                        Container {
                            horizontalAlignment: HorizontalAlignment.Right
                            verticalAlignment: VerticalAlignment.Center

                            Label {
                                text: ListItemData.time
                                textStyle { base: tsdxsmall.style }
                                //preferredWidth: 100
                            }
                        }                        
                        
                        gestureHandlers: [
                            TapHandler {
                                onTapped: {
                                    console.debug("Click on " + ListItemData.name);
                                }                                
                            }, 
                            LongPressHandler {
                                longPressed: {
                                    listItem.ListItem.view.setNumberToDial(ListItemData.number);
                                }                                
                            }
                        ]

                        attachedObjects: [
                            TextStyleDefinition {
                                id: tsdxlarge
                                base: SystemDefaults.TextStyles.BodyText
                                fontSize: FontSize.XLarge
                            },
                            TextStyleDefinition {
                                id: tsdxsmall
                                base: SystemDefaults.TextStyles.BodyText
                                fontSize: FontSize.XXSmall
                            }
                        ]//attachedObjects
                    }
                }
            ]
        }
    }//Container
}//Page
