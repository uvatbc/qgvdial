import bb.cascades 1.0

Container {
    id: container
    
    property string mainText
    property string subText
    
    signal clicked
    signal pressAndHold
    signal doubleClicked
    
    preferredWidth: 225
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
}
