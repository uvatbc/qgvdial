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
    id: container
    objectName: "DialPage"
    
    signal call(string num)
    signal text(string num)

    Container {
        attachedObjects: [
            TextStyleDefinition {
                id: tsd
                base: SystemDefaults.TextStyles.BodyText
                //fontWeight: FontWeight.Bold
                fontSize: FontSize.XXLarge
            }
        ] //attachedObjects
        
        DropDown {
            objectName: "RegNumberDropDown"
        }
        
        Container {
            layout: StackLayout {
                orientation: LayoutOrientation.RightToLeft
            }
            Button {
                text: "\u232B"
                onClicked: textNumber.text = textNumber.text.substr(0, textNumber.text.length-1);
                maxWidth: 130
                preferredHeight: 200
            }
            TextField {
                id: textNumber
                textStyle { base: tsd.style }
                preferredHeight: 200
                minWidth: 630
                inputMode: TextFieldInputMode.PhoneNumber
                onFocusedChanged: {
                    if (focused) {
                        keypad.visible = false;
                    } else {
                        keypad.visible = true;
                    }
                }
            }
        }
        
        Keypad {
            id: keypad
            onKeyPress: textNumber.text += text;
            onCall: container.call(textNumber.text);
            onText: container.text(textNumber.text);
        }
    }//Container
}//Page
