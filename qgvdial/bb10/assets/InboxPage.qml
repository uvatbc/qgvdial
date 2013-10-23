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
            signal setNumberToDial(string number);
            
            dataModel: inboxModel
            
            listItemComponents: [
                ListItemComponent {
                    Container {
                        id:  listItem
                        layout: StackLayout { orientation: LayoutOrientation.TopToBottom }

                        Container {
                            //layout: DockLayout {}
                            layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
                            layoutProperties: StackLayoutProperties { spaceQuota: 1 }
                            
                            Container {
                                verticalAlignment: VerticalAlignment.Center
                                
                                layout: StackLayout {
                                    orientation: LayoutOrientation.LeftToRight
                                }
                                preferredWidth: Infinity
                                
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
                                verticalAlignment: VerticalAlignment.Center
                
                                Label {
                                    text: ListItemData.time
                                    multiline: true
                                    textStyle { base: tsdxsmall.style }
                                    preferredWidth: 250
                                }
                            }                        
                            
                            gestureHandlers: [
                                TapHandler {
                                    onTapped: {
                                        console.debug("Click on " + ListItemData.name);
                                    }                                
                                }, 
                                LongPressHandler {
                                    onLongPressed: {
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
                                    textAlign: TextAlign.Right
                                }
                            ]//attachedObjects
                        }

                        Divider {
                            horizontalAlignment: HorizontalAlignment.Fill
                        }
                    }
                }
            ]
        }
    }//Container
}//Page
