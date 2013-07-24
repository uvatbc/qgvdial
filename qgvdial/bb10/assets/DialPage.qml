import bb.cascades 1.0

Page {
    id: container
    
    signal call(string num)
    signal text(string num)

    Container {
        Label {
            text: qsTr("Dial page")
            horizontalAlignment: HorizontalAlignment.Center
            textStyle {
                base: SystemDefaults.TextStyles.TitleText
            }
        }//Label
        
        Label {
            id: textNumber
        }
        
        Keypad {
            onKeyPress: textNumber.text += text;
            onDel: textNumber.text = textNumber.text.substr(0, textNumber.text.length-1);
            onCall: container.call(textNumber.text);
            onText: container.text(textNumber.text);
        }
    }//Container
}//Page
