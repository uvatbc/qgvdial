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
    
    property string mainText
    property string subText
    
    signal clicked
    signal pressAndHold
    signal doubleClicked
    
    preferredWidth: 240
    preferredHeight: 210
    
    layout: DockLayout { }

    attachedObjects: [
        TextStyleDefinition {
            id: tsdxsmall
            base: SystemDefaults.TextStyles.BodyText
            //fontWeight: FontWeight.Bold
            fontSize: FontSize.XSmall
        },
        TextStyleDefinition {
            id: tsdxxlarge
            base: SystemDefaults.TextStyles.BodyText
            fontWeight: FontWeight.Bold
            fontSize: FontSize.XXLarge
        }
    ]//attachedObjects
    
    gestureHandlers: [
        TapHandler {
            onTapped: {
                container.clicked();
            }
        },
        LongPressHandler {
            onLongPressed: {
                container.pressAndHold();
            }
        },
        DoubleTapHandler {
            onDoubleTapped: {
                container.doubleClicked();
            }
        }
    ]//gestureHandler
    
    Label {
        text: container.mainText 
        horizontalAlignment: HorizontalAlignment.Center
        verticalAlignment: VerticalAlignment.Center
        textStyle { base: tsdxxlarge.style }
    }
    
    Label {
        text: container.subText
        horizontalAlignment: HorizontalAlignment.Right
        verticalAlignment: VerticalAlignment.Center
        textStyle { base: tsdxsmall.style }
    }
    
    Divider {
        horizontalAlignment: HorizontalAlignment.Center
        verticalAlignment: VerticalAlignment.Bottom
    }
}
