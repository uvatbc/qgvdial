import bb.cascades 1.0

Container {
    id: container
    
    property string mainText
    property string subText
    
    signal clicked
    signal pressAndHold
    signal doubleClicked
    
    preferredWidth: 200
    preferredHeight: 200

    attachedObjects: [
        TextStyleDefinition {
            id: tsd
            base: SystemDefaults.TextStyles.BodyText
            //fontWeight: FontWeight.Bold
            fontSize: FontSize.Small
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
        verticalAlignment: VerticalAlignment.Top
    }
    
    Label {
        text: container.subText
        horizontalAlignment: HorizontalAlignment.Right
        verticalAlignment: VerticalAlignment.Bottom
        textStyle {
            base: tsd.style
        }
    }
}
