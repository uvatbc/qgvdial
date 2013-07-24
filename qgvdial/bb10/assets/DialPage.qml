import bb.cascades 1.0

Page {
    id: container
    
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
        
        Label {
            id: textNumber
            textStyle { base: tsd.style }
            preferredHeight: 200
        }
        
        Keypad {
            onKeyPress: textNumber.text += text;
            onDel: textNumber.text = textNumber.text.substr(0, textNumber.text.length-1);
            onCall: container.call(textNumber.text);
            onText: container.text(textNumber.text);
        }
    }//Container
}//Page
